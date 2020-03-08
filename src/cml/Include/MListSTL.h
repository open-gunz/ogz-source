// STL Support

#ifndef MLISTSTL_H
#define MLISTSTL_H

#include <list>
using namespace std;

// list<A*> 와 같은 형태로 쓰는 클래스
// 소멸자에서 자동으로 객체를 delete해준다.
template<class _Ty, class _A = allocator<_Ty> >
class MAutoDelPtrList : public list<_Ty, _A>{
public:
	void Erase(iterator _P){
		delete (*_P);
		erase(_P);
	}
	void Clear(void){
		while(begin()!=end()){
			delete *begin();	// 포인터형이여야 한다. 그렇지 않다면 컴파일러 에러가 난다.
			erase(begin());
		}
	}	
	virtual ~MAutoDelPtrList(void){
		Clear();
	}
};

#define FOR_BEGIN(_ClassName, _VarName)	{ for(_ClassName::iterator _i=_VarName.begin(); _i!=_VarName.end(); _i++){
#define FOR_EACH_ITOR()					(_i)
#define FOR_EACH_VALUE()				(*_i)
#define FOR_END()						}}

#endif
