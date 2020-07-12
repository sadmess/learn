#include <typeinfo>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#pragma warning(disable : 4996)
using namespace std;

class String
{
public:
    String()
        :_str(new char[1])
    {
        *_str = '\0';
    }
    String(const char* str)
        :_str(new char[strlen(str) + 1])
    {
        strcpy(this->_str, str);
    }
    String(const String& str)
        :_str(new char[strlen(str._str) + 1])
    {
        strcpy(this->_str, str._str);
    }
    ~String()
    {
        delete[]_str;
    }
    String& operator= (const String& str)
    {
        String tmp(str);
        
        swap(this->_str, tmp._str);
        tmp._str = nullptr;
        
        return *this;
    }
    void Display()
    {
        printf("%s\n", _str);
    }
public:
    char* _str;
};

String tmpstring("foo foo foo");

class foo {
public:
	int a;
	String b;
public:
	foo(const int& a ,const String& bb) {
		b=bb;
		this->a = a;
	}

	~foo() {
	}
};
/*void foolB(vector<foo>& fool) {
	for (int i = 0; i < 100; i++) {
        
		foo tmp(i, tmpstring);
		fool.push_back(tmp);
	}
}
void foolA(vector<foo>& fool) {
	foo& myfoo = fool.front();
	foolB(fool);

    myfoo.b.Display();
	cout << myfoo.a << endl;
}
int main() {
	foo fooA(1, tmpstring);
	vector<foo> kkk;
	kkk.push_back(fooA);
	foolA(kkk);

}*/
void remove(int pos, vector<int> vec) {
    vec.erase(vec.begin() + pos);
}
int main() {
    /*vector<foo> lll;
    int count = 0;
    for (int i = 0; i < 10; i++) {

        foo tmp(i, tmpstring);
        lll.push_back(tmp);
    }
    auto kkk = lll.begin();
    for (; kkk != lll.end(); ++kkk) {
        count++;
        if (count == 5) {

            foo tmp(1, tmpstring);
            lll.assign(3, tmp);
            //break;
        }
    }
    return 0;*/
    vector<int> lll;
    lll.push_back(1);
    remove(-1, lll);
    return 0;
}