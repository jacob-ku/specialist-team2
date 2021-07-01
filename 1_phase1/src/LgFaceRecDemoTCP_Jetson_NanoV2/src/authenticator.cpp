#include "authenticator.h"
#include "crypto_op.h"
#include "sslManager.h"
#include "Logger.h"


bool IsValidString(string str)
{
    // Check length
    if (str.empty() || str.length() > 16)
        return false;

    for (string::iterator it = str.begin(); it != str.end(); ++it)
    {
        if (!((*it >= 'a' && *it <= 'z') ||
            (*it >= 'A' && *it <= 'Z') ||
            (*it >= '0' && *it <= '9')))
        {
            return false;
        }
    }

    return true;
}

bool authenticateSystem()
{
    string passphrase = "";
    while (1)
    {
        cout << "Please enter system passphrase(): ";
        cin >> passphrase;
        if (IsValidString(passphrase)) break;
        else cout << "passphrase is invalid. Only number and alphabet will be accepted" << endl;
    }

    Authenticator auth = Authenticator();
    bool result = auth.validateSystem(passphrase);
    if (result == true)
    {
        LOGGER->Log("System login success.");
        crypto_op_init(passphrase.c_str(), passphrase.length());
        strncpy(sslManager_buf, passphrase.c_str(), sslManager_buf_size);
    } else
    {
        LOGGER->Log("System login failed.");
    }
    return result;
}

bool authenticateUser(unsigned char *idBuf, int idBufLen, unsigned char *pwdBuf, int pwdBufLen, AccessRight& access)
{
    try
    {
        Authenticator auth = Authenticator();
        string id = string(reinterpret_cast<char const*>(idBuf), idBufLen);
        string pwd = string(reinterpret_cast<char const*>(pwdBuf), pwdBufLen);

        if (!IsValidString(id) || !IsValidString(pwd))
        {
        	LOGGER->Log("User id/pwd is invalid");
            return false;
        }

        bool result = auth.validateUser(id, pwd, access);

        if (result == true) LOGGER->Log("User login success");
        else LOGGER->Log("User login failure");

        return result;
    } catch(std::exception e)
    {
        LOGGER->Log("Empty user id or password is not allowed.");
        return false;
    }
}

Authenticator::Authenticator()
{
    m_userCreds.push_back(
                {"admin",
                "bc95b39b902eb9ecac7ede0787f89a12e7b5c1ed81e545b55361c99e79ac1e3e",
                {0x5D, 0x81, 0x44, 0x85, 0x59, 0xAE, 0x06, 0x48,
                0xEC, 0x9C, 0xFF, 0x18, 0xD8, 0xAF, 0xFA, 0xD7}}
    );

    m_userCreds.push_back(
                {"kinduser",
                "c4dc8dfa00955fc789e095265f4a2773bee884195bc4ea1398316d19686fb211",
                {0xFF, 0xAF, 0x65, 0xB1, 0x89, 0x6A, 0x0D, 0x3C,
                0x38, 0x9B, 0x68, 0x1A, 0xAD, 0xC2, 0xC5, 0x90}}
    );
}

Authenticator::~Authenticator()
{
}

bool Authenticator::validateSystem(string passphrase)
{
    char *out_hexstr = new (nothrow) char[CREDENTIAL_LEN*2 + 1];
    if (out_hexstr == nullptr)
        return false;

    unsigned char *out_bin = new (nothrow) unsigned char[CREDENTIAL_LEN];
    if (out_bin == nullptr)
    {
        delete [] out_hexstr;
        return false;
    }

    pbkdf2_hmac_sha_512(passphrase.c_str(), passphrase.length(),
                        m_systemSalt, PBKDF2_SALT_LEN, PBKDF2_ITERTATION, CREDENTIAL_LEN,
                        out_hexstr, out_bin);

    if (m_allowedSystemCred.compare(out_hexstr) != 0)
        return false;

    delete [] out_hexstr;
    delete [] out_bin;
    return true;
}


bool Authenticator::validateUser(string user_id, string user_pwd, AccessRight& access)
{
    User person(user_id, user_pwd);

    bool result = false;

    char *out_hexstr = new (nothrow) char[CREDENTIAL_LEN*2 + 1];
    if (out_hexstr == nullptr)
        return false;

    unsigned char *out_bin = new (nothrow) unsigned char[CREDENTIAL_LEN];
    if (out_bin == nullptr)
    {
        delete [] out_hexstr;
        return false;
    }

    access = ACCESS_UNKNOWN;

    for (struct UserCredential userCred: m_userCreds)
    {
        if (userCred.id.compare(person.getId()) != 0)
            continue;

        pbkdf2_hmac_sha_512(person.getPwd().c_str(), person.getPwd().length(),
                            userCred.salt, PBKDF2_SALT_LEN, PBKDF2_ITERTATION, CREDENTIAL_LEN,
                            out_hexstr, out_bin);

        /* Correct password */
        if (userCred.cred.compare(out_hexstr) == 0)
        {
            if (userCred.id.compare("admin") == 0)
                access = ACCESS_ADMIN;
            else
                access = ACCESS_USER;
            result = true;
            break;
        }
    }

    delete [] out_hexstr;
    delete [] out_bin;

    return result;
}

