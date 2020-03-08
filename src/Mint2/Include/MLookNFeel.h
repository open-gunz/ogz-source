// Look & Feel macros
//
// In the definition of a class, DECLARE_LOOK declares static and non-static data members
// containing the look class, along with an OnDraw override, and functions for changing
// and retrieving the current look instance.
//
// DECLARE_LOOK_CLIENT declares an GetClientRect override that forwards to the look class.
//
// Note that the look class does not have to be defined at the point the class is parsed,
// although it does have to be declared (the macro does not declare it).
//
// In the translation unit that implements the class, IMPLEMENT_LOOK should be placed,
// which provides definitions of the static data members and non-inline member functions
// that were not defined in DECLARE_LOOK.
#pragma once

// Look & Feel declaration
//
// Declares variables: m_DefaultLook, m_pStaticLook, m_pCustomLook
// Declares function:  OnDraw (override of MWidget::OnDraw), ChangeLook, ChangeCustomLook
#define DECLARE_LOOK(CLASS_LOOK)								\
	private:													\
	static CLASS_LOOK	m_DefaultLook;							\
	static CLASS_LOOK*	m_pStaticLook;							\
	CLASS_LOOK*			m_pCustomLook{};						\
	protected:													\
	virtual void OnDraw(MDrawContext* pDC) override;			\
	public:														\
	static void ChangeLook(CLASS_LOOK* pLook);					\
	void ChangeCustomLook(CLASS_LOOK* pLook);					\
	CLASS_LOOK* GetLook(){										\
		if(m_pCustomLook!=NULL) return m_pCustomLook;			\
		return m_pStaticLook;									\
	}


// Look & Feel client information declaration
//
// Declares function: GetClientRect (override of MWidget::GetClientRect)
// The function forwards the call to a GetClientRect member function
// on the look class, which is expected to be binary and take
// the widget type and the base client rect as parameters.
#define DECLARE_LOOK_CLIENT()									\
	public:														\
	virtual MRECT GetClientRect() override {					\
		if(GetLook() == nullptr)								\
			return MWidget::GetClientRect();					\
		return GetLook()->GetClientRect(this,					\
			MWidget::GetClientRect());							\
	}


// Look & Feel implementation
//
// Implements static data members: m_pDefaultLook, m_pStaticLook
// Implements functions: OnDraw, ChangeLook, ChangeCustomLook
#define IMPLEMENT_LOOK(CLASS, CLASS_LOOK)						\
	CLASS_LOOK CLASS::m_DefaultLook;							\
	CLASS_LOOK* CLASS::m_pStaticLook = &CLASS::m_DefaultLook;	\
	void CLASS::OnDraw(MDrawContext* pDC){						\
		if(GetLook()!=NULL) GetLook()->OnDraw(this, pDC);		\
	}															\
	void CLASS::ChangeLook(CLASS_LOOK* pLook){					\
		if(pLook==NULL){										\
			m_pStaticLook = &m_DefaultLook;						\
			return;												\
		}														\
		m_pStaticLook = pLook;									\
	}															\
	void CLASS::ChangeCustomLook(CLASS_LOOK* pLook){			\
		m_pCustomLook = pLook;									\
	}
