# semant源码阅读
主要总结使用的两个类，感觉主要还是学习他的设计模式，感觉非常优秀（
## Environment类
每个类都有一个自己的Environment
以SymbolTable方式存储method与类内变量（包括但不限于attr）
```
SymbolTable<Symbol, method_class> method_table;
SymbolTable<Symbol, Entry>  var_table;
```

提供method table操作
```
//在methodtable添加method
void method_add(Symbol s, method_class *m);
//在mt查找方法
method_class *method_lookup(Symbol s);
//在顶层域查找方法
method_class *method_probe(Symbol s);
//进入新的方法域
void method_enterscope();
//销毁当前方法域
void method_exitscope();
```
提供 attribute table操作
```
//在at表添加变量
void var_add(Symbol s, Symbol typ);
//在at表中查找变量
Symbol var_lookup(Symbol s);
//在at表顶层域查找变量
Symbol var_probe(Symbol s);
//创建新的变量域
void var_enterscope();
//销毁当前变量域
void var_exitscope();
```
类间类型操作
```
//subtype是否是supertype子类，包括对SELF_TYPE的处理
int type_leq(Symbol subtype, Symbol supertype);
//查找t1 t2的最小共同祖先
Symbol type_lub(Symbol t1, Symbol t2);
```
## InheritanceNode类
继承关系中的节点，维护一个类是否可继承等变量，并维护一个子节点列表。
```
//检查是否为基础类
int basic() { return (basic_status == Basic); }
//是否可以继承
int inherit() { return (inherit_status == CanInherit); }
//标记是否可以通过继承关系遍历到，用于检测环形继承
void mark_reachable();
int reachable() { return (reach_status == Reachable); }
//给当前节点添加子节点（子类）
void add_child(InheritanceNodeP child);
//获取子节点列表
List<InheritanceNode> *get_children() { return children; }
//设置父节点（父类）
void set_parentnd(InheritanceNodeP p);
//得到父节点
InheritanceNodeP get_parentnd();
//从root节点开始递归对每一个类节点进行处理，向类节点中添加方法以及属性信息，重定义方法/属性名称的错误在这里被捕获。
void build_feature_tables();
//从root节点开始对所有类的exp进行递归检查
void type_check_features();
//检查main方法是否存在
void check_main_method();
```

# cgen源码阅读
## CgenClassTable
给每一个类设定一个tag整数来标记，介绍主要类方法与属性
### 类属性
```
用于类目与tag直接之间转换
SymbolTable<Symbol,int> class_to_tag_table;
SymbolTable<int,StringEntry> tag_to_class_table;

存储类节点
List<CgenNode> *nds;

类名查找它最小的儿子tag，用于检查一个类是否是一个类的子类
SymbolTable<Symbol,int> class_to_max_child_tag_table;

类名与方法表关联
SymbolTable<Symbol,SymbolTable<Symbol,int> > table_of_method_tables;
```
### 类方法
代码生成
```
全局数据段与全局代码段声明
void code_global_data();
void code_global_text();
声明gc，具体gc实现在trap.handle
void code_select_gc();
常量声明
void code_constants();
类名地址声明
void code_class_table();
```
编译期间
```
将类节点添加到列表
void install_basic_classes();
void install_class(CgenNodeP nd);
void install_classes(Classes cs);

生成继承树
void build_inheritance_tree();
void set_relations(CgenNodeP nd);

公有方法设置tag，对两个表都操作
int assign_tag(Symbol name);
设置最小继承与类名关系
void set_max_child(Symbol name, int tag);
设置类名与方法表关系
void add_method_table(Symbol name, SymbolTable<Symbol,int> *method_table);
```

## VarBinding
此类是对cool语言的变量类型进行操作

offset是表达如何取得此变量的属性，对于不同的子类有不同的使用方式。运行时有三种变量因此有三种子类。

### AttributeBinding
```
从s0寄存器即self处按offset取当前类的attr，offset需要加3，因为类有默认的数据结构
char* code_ref(char *optional_dest, ostream&);
更新类属性的值
void code_update(char *source, ostream&);
```
### SelfBinding
```
直接将s0寄存器移入dest
char* code_ref(char *optional_dest, ostream&);
```
### LocalBinding
```
offset负数代表存在寄存器中，整数代表在栈中存储

取当前值所在寄存器，若在栈中返回NULL
Register get_register();

更新值
void code_update(char *source, ostream&);

对值进行引用，如果是在寄存器第一个方法进行代码生成而是返回寄存器名称
char* code_ref(char *optional_dest, ostream&);
void code_ref_force_dest(char *dest, ostream& os);
```

## CgenNode
存储用于为一个类生成代码的全部信息
### 类属性
```
方法名映射offset
SymbolTable<Symbol,int> method_name_to_offset_table;
方法名映射一个方法需要预留的栈空间
SymbolTable<Symbol,int> method_name_to_numtemps_table;
方法offset映射methodbinding，md只存储了方法名与类名
SymbolTable<int,MethodBinding> method_offset_to_binding_table;

first_attribute存储在当前类中第一个属性的offset，num记录了总量，为了在继承关系中引用attr方便而如此设计
int first_attribute;
int num_attributes;
属性名映射变量binding
SymbolTable<Symbol,VarBinding> var_binding_table;
属性offset映射属性类，即code初始化属性相关信息
SymbolTable<int,attr_class>    attribute_init_table;
属性offset映射用于code属性原型的类
SymbolTable<int,Entry>         attribute_proto_table;
```

类方法
```
生成各种引用：方法调用，原型，init
void code_disptable_ref(ostream&);
void code_protoobj_ref(ostream&);
void code_init_ref(ostream&);
递归生成方法的代码的子方法
void code_method(ostream&, CgenEnvClassLevelP);

设置method相关映射
void layout_method(Symbol mname, int numtemps);
设置attribute相关映射
void layout_attribute(Symbol name, attr_class *a, int init);
生成类原型信息
void code_prototype_object(ostream&);
递归生成method
void code_methods(ostream&, CgenEnvTopLevelP);
生成类的init代码
void code_init(ostream&, CgenEnvTopLevelP);
生成方法调用表
void code_dispatch_table(ostream&);
```
## CgenEnvX
即不同级别的运行/编译期间环境
CgenEnvTopLevel（对应classtable）包括：
```
SymbolTable<Symbol,int> *class_to_tag_table;
SymbolTable<Symbol,int> *class_to_max_child_tag_table;
SymbolTable<Symbol,SymbolTable<Symbol,int> > *table_of_method_tables;
```
CgenEnvClassLevel(对应每个class)增加了:
```
SymbolTable<Symbol,VarBinding> var_binding_table;
Symbol classname; 
Symbol filename;  
```
CgenEnvironment（对应method）增加了：
```
static int next_label;  //用于生成一个代码跳转中独一无二的标签
int next_formal;        // 下一个合适的调用参数位置
int num_temps;          // 调用此方法需要的最大栈空间
int next_temp_location; // 下一个用于暂存变量的位置
```
