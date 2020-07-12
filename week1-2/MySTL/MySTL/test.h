#ifndef MYTINYSTL_TEST_H_
#define MYTINYSTL_TEST_H_

// һ���򵥵ĵ�Ԫ���Կ�ܣ������������� TestCase �� UnitTest���Լ�һϵ�����ڲ��Եĺ�

#include <ctime>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>


namespace MySTL
{
    namespace test
    {

#define green redbud::io::state::manual << redbud::io::hfg::green
#define red   redbud::io::state::manual << redbud::io::hfg::red

#if defined(_MSC_VER)
#pragma warning(disable : 4244)
#pragma warning(disable : 4996)
#endif

    } // namespace test

    namespace test
    {

        // TestCase ��
        // ��װ�������԰���
        class TestCase
        {
        public:
            // ���캯��������һ���ַ�������������
            TestCase(const char* case_name) : testcase_name(case_name) {}

            // һ�����麯�������ڲ��԰���
            virtual void Run() = 0;

        public:
            const char* testcase_name;  // ���԰���������
            int         nTestResult;    // ���԰�����ִ�н�� 
            double      nFailed;        // ����ʧ�ܵİ�����
            double      nPassed;        // ����ͨ���İ�����
        };

        // UnitTest ��
        // ��Ԫ���ԣ������в��԰������뵽 vector �У�����ִ�в��԰���
        class UnitTest
        {
        public:
            // ��ȡһ������
            static UnitTest* GetInstance();

            // ���������μ��� vector
            TestCase* RegisterTestCase(TestCase* testcase);

            void Run();

        public:
            TestCase* CurrentTestCase;          // ��ǰִ�еĲ��԰���
            double    nPassed;                  // ͨ��������
            double    nFailed;                  // ʧ�ܰ�����

        protected:
            std::vector<TestCase*> testcases_;  // ���永������
        };

        UnitTest* UnitTest::GetInstance()
        {
            static UnitTest instance;
            return &instance;
        }

        TestCase* UnitTest::RegisterTestCase(TestCase* testcase)
        {
            testcases_.push_back(testcase);
            return testcase;
        }

        void UnitTest::Run()
        {
            for (auto it : testcases_)
            {
                TestCase* testcase = it;
                CurrentTestCase = testcase;
                testcase->nTestResult = 1;
                testcase->nFailed = 0;
                testcase->nPassed = 0;
                std::cout  << "============================================\n";
                std::cout  << " Run TestCase:" << testcase->testcase_name << "\n";
                testcase->Run();
                std::cout << " " << testcase->nPassed << " / " << testcase->nFailed + testcase->nPassed
                    << " Cases passed. ( " << testcase->nPassed /
                    (testcase->nFailed + testcase->nPassed) * 100 << "% )\n";
                std::cout  << " End TestCase:" << testcase->testcase_name << "\n";
                if (testcase->nTestResult)
                    ++nPassed;
                else
                    ++nFailed;
            }
            std::cout  << "============================================\n";
            std::cout << " Total TestCase : " << nPassed + nFailed << "\n";
            std::cout << " Total Passed : " << nPassed << "\n";
            std::cout  << " Total Failed : " << nFailed << "\n";
            std::cout  << " " << nPassed << " / " << nFailed + nPassed
                << " TestCases passed. ( " << nPassed / (nFailed + nPassed) * 100 << "% )\n";
        }

        /*****************************************************************************************/

        // ���԰������������滻Ϊ test_cast_TEST
#define TESTCASE_NAME(testcase_name) \
    testcase_name##_TEST

// ʹ�ú궨���ڸǸ��ӵĲ���������װ���̣��� TEXT �еĲ��԰����ŵ���Ԫ������
#define MYTINYSTL_TEST_(testcase_name)                        \
class TESTCASE_NAME(testcase_name) : public TestCase {        \
public:                                                       \
    TESTCASE_NAME(testcase_name)(const char* case_name)       \
        : TestCase(case_name) {};                             \
    virtual void Run();                                       \
private:                                                      \
    static TestCase* const testcase_;                         \
};                                                            \
                                                              \
TestCase* const TESTCASE_NAME(testcase_name)                  \
    ::testcase_ = UnitTest::GetInstance()->RegisterTestCase(  \
        new TESTCASE_NAME(testcase_name)(#testcase_name));    \
void TESTCASE_NAME(testcase_name)::Run()

/*
Run()���û��дʵ�֣���Ϊ���ú궨�彫�����������뵽 Run ��ʵ������磺
TEST(AddTestDemo)
{
EXPECT_EQ(3, Add(1, 2));
EXPECT_EQ(2, Add(1, 1));
}
�������뽫 { EXPECT_EQ(3, Add(1, 2)); EXPECT_EQ(2, Add(1, 1)); } �ӵ� Run() �ĺ���
*/


/*****************************************************************************************/

// �򵥲��Եĺ궨��
// ���� : �궨����ʽΪ EXPECT_* ��������֤�����ģ���������ͨ��������ʧ��
// ʹ��һϵ�еĺ�����װ��֤��������Ϊ���¼����� :

/*
��ٶ���
EXPECT_TRUE  ��֤����: Condition Ϊ true
EXPECT_FALSE ��֤����: Condition Ϊ false

Example:
bool isPrime(int n);         һ���ж������ĺ���
EXPECT_TRUE(isPrime(2));     ͨ��
EXPECT_FALSE(isPrime(4));    ͨ��
EXPECT_TRUE(isPrime(6));     ʧ��
EXPECT_FALSE(isPrime(3));    ʧ��
*/
#define EXPECT_TRUE(Condition) do {                             \
  if (Condition) {                                              \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_TRUE succeeded!\n";          \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_TRUE failed!\n";               \
}} while(0)

#define EXPECT_FALSE(Condition) do {                            \
  if (!Condition) {                                             \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_FALSE succeeded!\n";         \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << "  EXPECT_FALSE failed!\n";             \
}} while(0)

/*
�Ƚ϶���
EXPECT_EQ(v1, v2) ��֤����: v1 == v2
EXPECT_NE(v1, v2) ��֤����: v1 != v2
EXPECT_LT(v1, v2) ��֤����: v1 <  v2
EXPECT_LE(v1, v2) ��֤����: v1 <= v2
EXPECT_GT(v1, v2) ��֤����: v1 >  v2
EXPECT_GE(v1, v2) ��֤����: v1 >= v2

Note:
1. ����Ӧ���� EXPECT_*(Expect, Actual)�ĸ�ʽ�����������ֵ���ұ���ʵ��ֵ
2. �ڶ���ʧ��ʱ���Ὣ����ֵ��ʵ��ֵ��ӡ����
3. ����ֵ�����ǿ�ͨ�����ԵıȽϲ��������бȽϵģ�����ֵ������֧�� << ��������
��ֵ���뵽 ostream ��
4. ��Щ���Կ��������û��Զ����ͱ𣬵�����������Ӧ�ıȽϲ��������� == ��< �ȣ�
5. EXPECT_EQ ��ָ����е��ǵ�ַ�Ƚϡ����Ƚϵ��������Ƿ�ָ����ͬ���ڴ��ַ��
����������ָ��������Ƿ���ȡ������Ƚ����� C �ַ���(const char*)��ֵ��
��ʹ�� EXPECT_STREQ ���ر�һ����ǣ�Ҫ��֤һ�� C �ַ����Ƿ�Ϊ��(NULL)��
��ʹ�� EXPECT_STREQ(NULL, c_str)������Ҫ�Ƚ����� string ����ʱ��
Ӧ��ʹ�� EXPECT_EQ

Example:
EXPECT_EQ(3, foo());
EXPECT_NE(NULL, pointer);
EXPECT_LT(len, v.size());
*/
#define EXPECT_EQ(v1, v2) do { \
  if (v1 == v2) {                                               \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_EQ succeeded!\n";            \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_EQ failed!\n";                 \
    std::cout << red << " Expect:" << v1 << "\n";               \
    std::cout << red << " Actual:" << v2 << "\n";               \
}} while(0)

#define EXPECT_NE(v1, v2) do {                                  \
  if (v1 != v2) {                                               \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
   std::cout << green << " EXPECT_NE succeeded!\n";             \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_NE failed!\n";                 \
    std::cout << red << " Expect:" << v1 << "\n";               \
    std::cout << red << " Actual:" << v2 << "\n";               \
}} while(0)

#define EXPECT_LT(v1, v2) do {                                  \
  if (v1 < v2) {                                                \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_LT succeeded!\n";            \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_LT failed!\n";                 \
    std::cout << red << " Expect:" << v1 << "\n";               \
    std::cout << red << " Actual:" << v2 << "\n";               \
}} while(0)

#define EXPECT_LE(v1, v2) do {                                  \
  if (v1 <= v2) {                                               \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_LE succeeded!\n";            \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_LE failed!\n";                 \
    std::cout << red << " Expect:" << v1 << "\n";               \
    std::cout << red << " Actual:" << v2 << "\n";               \
}} while(0)

#define EXPECT_GT(v1, v2) do {                                  \
  if (v1 > v2) {                                                \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_GT succeeded!\n";            \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_GT failed!\n";                 \
    std::cout << red << " Expect:" << v1 << "\n";               \
    std::cout << red << " Actual:" << v2 << "\n";               \
}} while(0)

#define EXPECT_GE(v1, v2) do {                                  \
  if (v1 >= v2) {                                               \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_GE succeeded!\n";            \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_GE failed!\n";                 \
    std::cout << red << " Expect:" << v1 << "\n";               \
    std::cout << red << " Actual:" << v2 << "\n";               \
}} while(0)

/*
�ַ����Ƚ�
EXPECT_STREQ(s1, s2) ��֤����: ���� C �ַ�������ͬ��ֵ
EXPECT_STRNE(s1, s2) ��֤����: ���� C �ַ����в�ͬ��ֵ

Note:
1. ����Ӧ���� EXPECT_STR*(Expect, Actual)�ĸ�ʽ�����������ֵ���ұ���ʵ��ֵ
2. ����������ڱȽ����� C �ַ������������Ҫ�Ƚ����� string ������Ӧ��ʹ��
EXPECT_EQ��EXPECT_NE �ȶ���
3. EXPECT_STREQ �� EXPECT_STRNE �����ܿ��ַ�����wchar_t*��
4. һ�� NULL ָ���һ�����ַ����᲻��һ����

Example:
char* s1 = "", char* s2 = "abc", char* s3 = NULL;
EXPECT_STREQ("abc", s2);  ͨ��
EXPECT_STREQ(s1, s3);     ʧ��
EXPECT_STREQ(NULL, s3);   ͨ��
EXPECT_STRNE(" ", s1);    ͨ��
*/

#define EXPECT_STREQ(s1, s2) do {                                 \
  if (s1 == NULL || s2 == NULL) {                                 \
    if (s1 == NULL && s2 == NULL) {                               \
      UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
      std::cout << green << " EXPECT_STRED succeeded!\n";         \
    }                                                             \
    else {                                                        \
      UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
      UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
      std::cout << red << " EXPECT_STRED failed!\n";              \
      if(s1 == NULL) std::cout << " Expect: NULL\n";              \
      else std::cout << " Expect:\"" << s1 << "\"\n";             \
      if(s2 == NULL) std::cout << " Actual: NULL\n";              \
      else std::cout << " Actual:\"" << s2 << "\"\n";             \
    }                                                             \
  }                                                               \
  else if (strcmp(s1, s2) == 0) {                                 \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;          \
    std::cout << green << " EXPECT_STRED succeeded!\n";           \
  }                                                               \
  else {                                                          \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;    \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;          \
    std::cout << red << " EXPECT_STRED failed!\n";                \
    std::cout << red << " Expect:\"" << s1 << "\"\n";             \
    std::cout << red << " Actual:\"" << s2 << "\"\n";             \
}} while(0)

#define EXPECT_STRNE(s1, s2) do {                                 \
  if (s1 == NULL || s2 == NULL) {                                 \
    if (s1 != NULL || s2 != NULL) {                               \
      UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
      std::cout << green << " EXPECT_STRNE succeeded!\n";         \
    }                                                             \
    else {                                                        \
      UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
      UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
      std::cout << red << " EXPECT_STRNE failed!\n";              \
      if(s1 == NULL) std::cout << " Expect: NULL\n";              \
      else std::cout << " Expect:\"" << s1 << "\"\n";             \
      if(s2 == NULL) std::cout << " Actual: NULL\n";              \
      else std::cout << " Actual:\"" << s2 << "\"\n";             \
    }                                                             \
  }                                                               \
  else if (strcmp(s1, s2) != 0) {                                 \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;          \
    std::cout << green << " EXPECT_STRNE succeeded!\n";           \
  }                                                               \
  else {                                                          \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;    \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;          \
    std::cout << red << " EXPECT_STRNE failed!\n";                \
    std::cout << red << " Expect:\"" << s1 << "\"\n";             \
    std::cout << red << " Actual:\"" << s2 << "\"\n";             \
}} while(0)

/*
ָ��Ƚ�
EXPECT_PTR_EQ(p1, p2)            ��֤����: *p1 == *p2
EXPECT_PTR_NE(p1, p2)            ��֤����: *p1 != *p2
EXPECT_PTR_RANGE_EQ(p1, p2, len) ��֤����: ���� i (*p1 + i) == (*p2 + i)  i��[0,len)
EXPECT_PTR_RANGE_NE(p1, p2, len) ��֤����: ���� i (*p1 + i) != (*p2 + i)  i��[0,len)

Note:
1. ����Ӧ���� EXPECT_PTR_*(Expect, Actual)��
EXPECT_PTR_RANGE_*(Expect, Actual, len)�ĸ�ʽ��
��������������ֵ��ʵ��ֵ���
2. EXPECT_PTR_EQ �Ƚϵ���ָ����ָԪ�ص�ֵ�����Ҫ�Ƚ�
ָ��ָ��ĵ�ַ�Ƿ���ȣ����� EXPECT_EQ
3. EXPECT_PTR_RANGE_* �Ƚϵ��Ǵ� p1��p2 ��ʼ��
����Ϊ len �����䣬��ȷ�����䳤����Ч

Example:
int a[] = {1,2,3,4,5};
int b[] = {1,2,3,4,6};
int *p1 = a, *p2 = b;
EXPECT_PTR_EQ(p1, p2);                      ͨ��
p1 = a + 4, p2 = b + 4;
EXPECT_PTR_EQ(p1, p2);                      ʧ��
EXPECT_PTR_EQ(p1, std::find(a, a + 5, 5));  ͨ��
EXPECT_PTR_RANGE_EQ(a, b, 5);               ʧ��
EXPECT_PTR_RANGE_EQ(a, b, 4);               ͨ��
*/
#define EXPECT_PTR_EQ(p1, p2) do {                              \
  if (*p1 == *p2) {                                             \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_PTR_EQ succeeded!\n";        \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_PTR_EQ failed!\n";             \
    std::cout << red << " Expect:" << *p1 << "\n";              \
    std::cout << red << " Actual:" << *p2 << "\n";              \
}} while(0)

#define EXPECT_PTR_NE(p1, p2) do {                              \
  if (*p1 != *p2) {                                             \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_PTR_NE succeeded!\n";        \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_PTR_NE failed!\n";             \
    std::cout << red << " Expect:" << *p1 << "\n";              \
    std::cout << red << " Actual:" << *p2 << "\n";              \
}} while(0)

#define EXPECT_PTR_RANGE_EQ(p1, p2, len) do {                   \
  if (std::equal(p1, p1 + len, p2)) {                           \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_PTR_RANGE_EQ succeeded!\n";  \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_PTR_RANGE_EQ failed!\n";       \
}} while(0)

#define EXPECT_PTR_RANGE_NE(p1, p2, len) do {                   \
  if (!std::equal(p1, p1 + len, p2)) {                          \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;        \
    std::cout << green << " EXPECT_PTR_RANGE_NE succeeded!\n";  \
  }                                                             \
  else {                                                        \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;  \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;        \
    std::cout << red << " EXPECT_PTR_RANGE_NE failed!\n";       \
}} while(0)

/*
�����Ƚ�
EXPECT_CON_EQ(c1, c2) ��֤����: c1 == c2
EXPECT_CON_NE(c1, c2) ��֤����: c1 != c2

Note:
1. ���������� STL �������Զ�����������������飬����������ָ��
2. ��������������Ҫ�ܹ����бȽϣ�����һ�»���Է�����ʽת��
3. EXPECT_CON_EQ ����ʧ��ʱ�����ӡ�״β���ȵ�����ֵ

Example:
int arr[] = {1,2,3};
std::vector<int> v1{1, 2, 3};
std::vector<int> v2{2, 3, 4};
MySTL::vector<long> v3(arr, arr + 3);
EXPECT_CON_NE(v1, v2)   ok
EXPECT_CON_EQ(arr, v1)  ok
EXPECT_CON_EQ(v1, v3)   ok
*/
#define EXPECT_CON_EQ(c1, c2) do {                                  \
  auto first1 = std::begin(c1), last1 = std::end(c1);               \
  auto first2 = std::begin(c2), last2 = std::end(c2);               \
  for (; first1 != last1 && first2 != last2; ++first1, ++first2) {  \
    if (*first1 != *first2)  break;                                 \
  }                                                                 \
  if (first1 == last1 && first2 == last2) {                         \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;            \
    std::cout << green << " EXPECT_CON_EQ succeeded!\n";            \
  }                                                                 \
  else {                                                            \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;      \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;            \
    std::cout << red << " EXPECT_CON_EQ failed!\n";                 \
    std::cout << red << " Expect:" << *first1 << "\n";              \
    std::cout << red << " Actual:" << *first2 << "\n";              \
}} while(0)

#define EXPECT_CON_NE(c1, c2) do {                                  \
  auto first1 = std::begin(c1), last1 = std::end(c1);               \
  auto first2 = std::begin(c2), last2 = std::end(c2);               \
  for (; first1 != last1 && first2 != last2; ++first1, ++first2) {  \
    if (*first1 != *first2)  break;                                 \
  }                                                                 \
  if (first1 != last1 || first2 != last2) {                         \
    UnitTest::GetInstance()->CurrentTestCase->nPassed++;            \
    std::cout << green << " EXPECT_CON_NE succeeded!\n";            \
  }                                                                 \
  else {                                                            \
    UnitTest::GetInstance()->CurrentTestCase->nTestResult = 0;      \
    UnitTest::GetInstance()->CurrentTestCase->nFailed++;            \
    std::cout << red << " EXPECT_CON_NE failed!\n";                 \
}} while(0)

/*****************************************************************************************/
// ���õĺ궨��

// ��ͬ����Ĳ���������
#if defined(_DEBUG) || defined(DEBUG)
#define LEN1    10000
#define LEN2    100000
#define LEN3    1000000
#else
#define LEN1    100000
#define LEN2    1000000
#define LEN3    10000000
#endif

#define _LLL * 20
#define _LL  * 10
#define _L   * 5
#define _M
#define _S   / 5
#define _SS  / 10
#define _SSS / 20

#define WIDE    14

// ���ͨ����ʾ
#define PASSED    std::cout << "[ PASSED ]\n"

// �����������
#define COUT(container) do {                             \
  std::string con_name = #container;                     \
  std::cout << " " << con_name << " :";                  \
  for (auto it : container)                              \
    std::cout << " " << it;                              \
  std::cout << "\n";                                     \
} while(0)

#define STR_COUT(str) do {                               \
  std::string str_name = #str;                           \
  std::cout << " " << str_name << " : " << str << "\n";  \
} while(0)

// ����������ú�����Ľ��
#define FUN_AFTER(con, fun) do {                         \
  std::string fun_name = #fun;                           \
  std::cout << " After " << fun_name << " :\n";          \
  fun;                                                   \
  COUT(con);                                             \
} while(0)

#define STR_FUN_AFTER(str, fun) do {                     \
  std::string fun_name = #fun;                           \
  std::cout << " After " << fun_name << " :\n";          \
  fun;                                                   \
  STR_COUT(str);                                         \
} while(0)

// ����������ú�����ֵ
#define FUN_VALUE(fun) do {                              \
  std::string fun_name = #fun;                           \
  std::cout << " " << fun_name << " : " << fun << "\n";  \
} while(0)

// �������������
        void test_len(size_t len1, size_t len2, size_t len3, size_t wide)
        {
            std::string str1, str2, str3;
            std::stringstream ss;
            ss << len1 << " " << len2 << " " << len3;
            ss >> str1 >> str2 >> str3;
            str1 += "   |";
            std::cout << std::setw(wide) << str1;
            str2 += "   |";
            std::cout << std::setw(wide) << str2;
            str3 += "   |";
            std::cout << std::setw(wide) << str3 << "\n";
        }

#define TEST_LEN(len1, len2, len3, wide) \
  test_len(len1, len2, len3, wide)

        // ���ò������ܵĺ�
#define FUN_TEST_FORMAT1(mode, fun, arg, count) do {         \
  srand((int)time(0));                                       \
  clock_t start, end;                                        \
  mode c;                                                    \
  char buf[10];                                              \
  start = clock();                                           \
  for (size_t i = 0; i < count; ++i)                         \
    c.fun(arg);                                              \
  end = clock();                                             \
  int n = static_cast<int>(static_cast<double>(end - start)  \
      / CLOCKS_PER_SEC * 1000);                              \
  std::snprintf(buf, sizeof(buf), "%d", n);                  \
  std::string t = buf;                                       \
  t += "ms    |";                                            \
  std::cout << std::setw(WIDE) << t;                         \
} while(0)

#define FUN_TEST_FORMAT2(mode, fun, arg1, arg2, count) do {  \
  srand((int)time(0));                                       \
  clock_t start, end;                                        \
  mode c;                                                    \
  char buf[10];                                              \
  start = clock();                                           \
  for (size_t i = 0; i < count; ++i)                         \
    c.fun(c.arg1(), arg2);                                   \
  end = clock();                                             \
  int n = static_cast<int>(static_cast<double>(end - start)  \
      / CLOCKS_PER_SEC * 1000);                              \
  std::snprintf(buf, sizeof(buf), "%d", n);                  \
  std::string t = buf;                                       \
  t += "ms    |";                                            \
  std::cout << std::setw(WIDE) << t;                         \
} while(0)

#define LIST_SORT_DO_TEST(mode, count) do {                  \
  srand((int)time(0));                                       \
  clock_t start, end;                                        \
  mode::list<int> l;                                         \
  char buf[10];                                              \
  for (size_t i = 0; i < count; ++i)                         \
    l.insert(l.end(), rand());                               \
  start = clock();                                           \
  l.sort();                                                  \
  end = clock();                                             \
  int n = static_cast<int>(static_cast<double>(end - start)  \
      / CLOCKS_PER_SEC * 1000);                              \
  std::snprintf(buf, sizeof(buf), "%d", n);                  \
  std::string t = buf;                                       \
  t += "ms    |";                                            \
  std::cout << std::setw(WIDE) << t;                         \
} while(0)

#define MAP_EMPLACE_DO_TEST(mode, con, count) do {           \
  srand((int)time(0));                                       \
  clock_t start, end;                                        \
  mode::con<int, int> c;                                     \
  char buf[10];                                              \
  start = clock();                                           \
  for (size_t i = 0; i < count; ++i)                         \
    c.emplace(mode::make_pair(rand(), rand()));              \
  end = clock();                                             \
  int n = static_cast<int>(static_cast<double>(end - start)  \
      / CLOCKS_PER_SEC * 1000);                              \
  std::snprintf(buf, sizeof(buf), "%d", n);                  \
  std::string t = buf;                                       \
  t += "ms    |";                                            \
  std::cout << std::setw(WIDE) << t;                         \
} while(0)

// �ع��ظ�����
#define CON_TEST_P1(con, fun, arg, len1, len2, len3)         \
  std::cout << "|         std         |";                    \
  FUN_TEST_FORMAT1(std::con, fun, arg, len1);                \
  FUN_TEST_FORMAT1(std::con, fun, arg, len2);                \
  FUN_TEST_FORMAT1(std::con, fun, arg, len3);                \
  std::cout << "\n|        MySTL        |";                  \
  FUN_TEST_FORMAT1(MySTL::con, fun, arg, len1);              \
  FUN_TEST_FORMAT1(MySTL::con, fun, arg, len2);              \
  FUN_TEST_FORMAT1(MySTL::con, fun, arg, len3);    

#define CON_TEST_P2(con, fun, arg1, arg2, len1, len2, len3)  \
  std::cout << "|         std         |";                    \
  FUN_TEST_FORMAT2(std::con, fun, arg1, arg2, len1);         \
  FUN_TEST_FORMAT2(std::con, fun, arg1, arg2, len2);         \
  FUN_TEST_FORMAT2(std::con, fun, arg1, arg2, len3);         \
  std::cout << "\n|        MySTL        |";                  \
  FUN_TEST_FORMAT2(MySTL::con, fun, arg1, arg2, len1);       \
  FUN_TEST_FORMAT2(MySTL::con, fun, arg1, arg2, len2);       \
  FUN_TEST_FORMAT2(MySTL::con, fun, arg1, arg2, len3);    

#define MAP_EMPLACE_TEST(con, len1, len2, len3)              \
  TEST_LEN(len1, len2, len3, WIDE);                          \
  std::cout << "|         std         |";                    \
  MAP_EMPLACE_DO_TEST(std, con, len1);                       \
  MAP_EMPLACE_DO_TEST(std, con, len2);                       \
  MAP_EMPLACE_DO_TEST(std, con, len3);                       \
  std::cout << "\n|        MySTL        |";                  \
  MAP_EMPLACE_DO_TEST(MySTL, con, len1);                     \
  MAP_EMPLACE_DO_TEST(MySTL, con, len2);                     \
  MAP_EMPLACE_DO_TEST(MySTL, con, len3);

#define LIST_SORT_TEST(len1, len2, len3)                     \
  std::cout << "|         std         |";                    \
  LIST_SORT_DO_TEST(std, len1);                              \
  LIST_SORT_DO_TEST(std, len2);                              \
  LIST_SORT_DO_TEST(std, len3);                              \
  std::cout << "\n|        MySTL        |";                  \
  LIST_SORT_DO_TEST(MySTL, len1);                            \
  LIST_SORT_DO_TEST(MySTL, len2);                            \
  LIST_SORT_DO_TEST(MySTL, len3);

// �򵥲��Եĺ궨��
#define TEST(testcase_name) \
  MYTINYSTL_TEST_(testcase_name)

// �������в��԰���
#define RUN_ALL_TESTS() \
  MySTL::test::UnitTest::GetInstance()->Run()

// �Ƿ������ܲ���
#ifndef PERFORMANCE_TEST_ON
#define PERFORMANCE_TEST_ON 1
#endif // !PERFORMANCE_TEST_ON

// �Ƿ���������������
#ifndef LARGER_TEST_DATA_ON
#define LARGER_TEST_DATA_ON 0
#endif // !LARGER_TEST_DATA_ON

    }    // namespace test
}    // namespace MySTL
#endif // !MYTINYSTL_TEST_H_

