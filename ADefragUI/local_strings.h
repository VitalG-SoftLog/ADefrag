#if !defined(LOCAL_STRINGS_INCLUDE)
#define LOCAL_STRINGS_INCLUDE

#pragma message("Localization string identifiers included")

class CLocalString : public CString
{
public:
    CLocalString( UINT id ) { LoadString( id ); }
    virtual ~CLocalString() {}
};

#define BASEID 34000

//#define IDS_ABOUT_EMAIL_ADDRESS                         (BASEID+0)

#endif // LOCAL_STRINGS_INCLUDE
