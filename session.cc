#include <cstdlib>
#include "pkcs11test.h"

using namespace std;  // So sue me

namespace pkcs11 {
namespace test {

TEST_F(PKCS11Test, ParallelSessionUnsupported) {
  // No CKF_SERIAL_SESSION => not supported
  CK_SESSION_HANDLE session;
  EXPECT_CKR(CKR_SESSION_PARALLEL_NOT_SUPPORTED, g_fns->C_OpenSession(g_slot_id, 0, NULL_PTR, NULL_PTR, &session));
}

TEST_F(ReadOnlySessionTest, SessionInfo) {
  CK_SESSION_INFO session_info;
  EXPECT_CKR_OK(g_fns->C_GetSessionInfo(session_, &session_info));
  if (g_verbose) cout << session_info_description(&session_info) << endl;
}

TEST_F(ReadWriteSessionTest, SessionInfo) {
  CK_SESSION_INFO session_info;
  EXPECT_CKR_OK(g_fns->C_GetSessionInfo(session_, &session_info));
  if (g_verbose) cout << session_info_description(&session_info) << endl;
}

TEST_F(ReadWriteSessionTest, GetSetOperationState) {
  CK_ULONG len;
  CK_RV rv = g_fns->C_GetOperationState(session_, NULL_PTR, &len);
  if (rv == CKR_FUNCTION_NOT_SUPPORTED) return;

  // No state => OPERATION_NOT_INITIALIZED
  EXPECT_CKR(CKR_OPERATION_NOT_INITIALIZED, rv);

  // Create some state
  vector<CK_ATTRIBUTE_TYPE> attrs({CKA_ENCRYPT, CKA_DECRYPT});
  SecretKey key(session_, attrs);

  CK_MECHANISM mechanism = {CKM_DES_ECB, NULL_PTR, 0};
  rv = g_fns->C_EncryptInit(session_, &mechanism, key.handle());
  EXPECT_CKR_OK(rv);
  if (rv != CKR_OK) return;

  rv = g_fns->C_GetOperationState(session_, NULL_PTR, &len);
  if (rv != CKR_STATE_UNSAVEABLE) {
    EXPECT_CKR(CKR_BUFFER_TOO_SMALL, rv);
    unique_ptr<CK_BYTE, freer> state((CK_BYTE*)malloc(len));
    rv = g_fns->C_GetOperationState(session_, state.get(), &len);
    EXPECT_CKR_OK(rv);
    if (rv == CKR_OK) {
      EXPECT_CKR_OK(g_fns->C_SetOperationState(session_, state.get(), len, 0, 0));
  }
  }
}

}  // namespace test
}  // namespace pkcs11
