#pragma once
#include "Types.h"
#include <windows.h>
#include "DBBind.h"

namespace SP
{
	
    class SignUp : public DBBind<2,0>
    {
    public:
    	SignUp(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSignUp(?,?)}") { }
    	template<int32 N> void In_LoginId(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void In_LoginId(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void In_LoginId(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_LoginId(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	template<int32 N> void In_Password(WCHAR(&v)[N]) { BindParam(1, v); };
    	template<int32 N> void In_Password(const WCHAR(&v)[N]) { BindParam(1, v); };
    	void In_Password(WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void In_Password(const WCHAR* v, int32 count) { BindParam(1, v, count); };

    private:
    };

    class GetAccountId : public DBBind<2,1>
    {
    public:
    	GetAccountId(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetAccountId(?,?)}") { }
    	template<int32 N> void In_LoginId(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void In_LoginId(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void In_LoginId(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_LoginId(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	template<int32 N> void In_Password(WCHAR(&v)[N]) { BindParam(1, v); };
    	template<int32 N> void In_Password(const WCHAR(&v)[N]) { BindParam(1, v); };
    	void In_Password(WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void In_Password(const WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void Out_AccountId(OUT int32& v) { BindCol(0, v); };

    private:
    };

    class GetCharacters : public DBBind<1,2>
    {
    public:
    	GetCharacters(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetCharacters(?)}") { }
    	void In_AccountId(int32& v) { BindParam(0, v); };
    	void In_AccountId(int32&& v) { _accountId = std::move(v); BindParam(0, _accountId); };
    	void Out_JobType(OUT int32& v) { BindCol(0, v); };
    	template<int32 N> void Out_CharacterName(OUT WCHAR(&v)[N]) { BindCol(1, v); };

    private:
    	int32 _accountId = {};
    };

    class CreateNewCharacter : public DBBind<3,0>
    {
    public:
    	CreateNewCharacter(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spCreateNewCharacter(?,?,?)}") { }
    	void In_AccountId(int32& v) { BindParam(0, v); };
    	void In_AccountId(int32&& v) { _accountId = std::move(v); BindParam(0, _accountId); };
    	void In_Type(int32& v) { BindParam(1, v); };
    	void In_Type(int32&& v) { _type = std::move(v); BindParam(1, _type); };
    	template<int32 N> void In_Name(WCHAR(&v)[N]) { BindParam(2, v); };
    	template<int32 N> void In_Name(const WCHAR(&v)[N]) { BindParam(2, v); };
    	void In_Name(WCHAR* v, int32 count) { BindParam(2, v, count); };
    	void In_Name(const WCHAR* v, int32 count) { BindParam(2, v, count); };

    private:
    	int32 _accountId = {};
    	int32 _type = {};
    };

    class GetItems : public DBBind<1,2>
    {
    public:
    	GetItems(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetItems(?)}") { }
    	template<int32 N> void In_Name(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void In_Name(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void In_Name(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_Name(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void Out_ItemId(OUT int32& v) { BindCol(0, v); };
    	void Out_Count(OUT int32& v) { BindCol(1, v); };

    private:
    };

    class UpdateItem : public DBBind<3,0>
    {
    public:
    	UpdateItem(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spUpdateItem(?,?,?)}") { }
    	template<int32 N> void In_Name(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void In_Name(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void In_Name(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_Name(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_ItemId(int32& v) { BindParam(1, v); };
    	void In_ItemId(int32&& v) { _itemId = std::move(v); BindParam(1, _itemId); };
    	void In_Count(int32& v) { BindParam(2, v); };
    	void In_Count(int32&& v) { _count = std::move(v); BindParam(2, _count); };

    private:
    	int32 _itemId = {};
    	int32 _count = {};
    };

    class InsertItem : public DBBind<3,0>
    {
    public:
    	InsertItem(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spInsertItem(?,?,?)}") { }
    	template<int32 N> void In_Name(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void In_Name(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void In_Name(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_Name(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_ItemId(int32& v) { BindParam(1, v); };
    	void In_ItemId(int32&& v) { _itemId = std::move(v); BindParam(1, _itemId); };
    	void In_Count(int32& v) { BindParam(2, v); };
    	void In_Count(int32&& v) { _count = std::move(v); BindParam(2, _count); };

    private:
    	int32 _itemId = {};
    	int32 _count = {};
    };

    class GetMoney : public DBBind<1,1>
    {
    public:
    	GetMoney(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetMoney(?)}") { }
    	template<int32 N> void In_Name(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void In_Name(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void In_Name(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_Name(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void Out_Value(OUT int32& v) { BindCol(0, v); };

    private:
    };


     
};