//------------------------------------------------------------------------------------------------
// File: authenticator.h
// Project: LG Security Specialist Program
// Versions:
// 1.0 June 2021 - initial version
//------------------------------------------------------------------------------------------------

#ifndef __AUTHENTICATOR_H__
#define __AUTHENTICATOR_H__

#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct UserCredential
{
    string id;
    string cred;
    unsigned char salt[16];
};

enum AccessRight
{
    ACCESS_UNKNOWN,
    ACCESS_ADMIN,
    ACCESS_USER
};

class Authenticator
{
    public:
        Authenticator();
        ~Authenticator();

        bool validateUser(string user_id, string user_pwd, AccessRight& accessRight);
        bool validateSystem(string passphrase);
    private:
        class User {
            public:
                User(string id, string pwd) : m_id(id), m_pwd(pwd) {
                    if (id.empty() || pwd.empty())
                        throw std::exception();
                };
                ~User() {};

                string getId()              { return m_id; }
                string getPwd()         { return m_pwd; }
            private:
                string m_id;
                string m_pwd;
        };


        const unsigned int CREDENTIAL_LEN = 32;

        // static int m_attemptCount;
        vector<struct UserCredential> m_userCreds;
        const string m_allowedSystemCred = "6cc6f821a1b009dd110d1d6ae9233d60205bac93db662f167858bbda48997c01";
        const unsigned char m_systemSalt[16] = {0x62, 0xA4, 0x66, 0x5E, 0x21, 0xEB, 0xA1, 0x40,
                                                0x6B, 0xEC, 0xD6, 0xDA, 0x46, 0x11, 0x90, 0xF8};
};

bool authenticateUser(unsigned char *idBuf, int idBufLen, unsigned char *pwdBuf, int pwdBufLen, AccessRight& accessRight);
bool authenticateSystem();
bool IsValidString(string str);

#endif //__AUTHENTICATOR_H__

