
#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_crypto_c,"$Id: w32_crypto.c,v 1.1 2018/10/18 00:38:55 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * crypt32.dll dynamic loader ...
 *
 * Copyright (c) 2017 - 2018, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#define  WINDOWS_MEAN_AND_LEAN
#include <windows.h>
#include <bcrypt.h>

#include <stdio.h>

typedef NTSTATUS WINAPI (* BCryptOpenAlgorithmProvider_t)(BCRYPT_ALG_HANDLE *phAlgorithm, LPCWSTR pszAlgId, LPCWSTR pszImplementation, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptEnumAlgorithms_t)(ULONG dwAlgOperations, ULONG *pAlgCount, BCRYPT_ALGORITHM_IDENTIFIER **ppAlgList, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptEnumProviders_t)(LPCWSTR pszAlgId, ULONG *pImplCount, BCRYPT_PROVIDER_NAME **ppImplList, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptGetProperty_t)(BCRYPT_HANDLE hObject, LPCWSTR pszProperty, PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptSetProperty_t)(BCRYPT_HANDLE hObject, LPCWSTR pszProperty, PUCHAR pbInput, ULONG cbInput, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptCloseAlgorithmProvider_t)(BCRYPT_ALG_HANDLE hAlgorithm, ULONG dwFlags);
typedef VOID     WINAPI (* BCryptFreeBuffer_t)(PVOID pvBuffer);
typedef NTSTATUS WINAPI (* BCryptGenerateSymmetricKey_t)(BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_KEY_HANDLE *phKey, PUCHAR pbKeyObject, ULONG cbKeyObject, PUCHAR pbSecret, ULONG cbSecret, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptGenerateKeyPair_t)(BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_KEY_HANDLE *phKey, ULONG dwLength, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptEncrypt_t)(BCRYPT_KEY_HANDLE hKey, PUCHAR pbInput, ULONG cbInput, VOID *pPaddingInfo, PUCHAR pbIV, ULONG cbIV, PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptDecrypt_t)(BCRYPT_KEY_HANDLE hKey, PUCHAR pbInput, ULONG cbInput, VOID *pPaddingInfo, PUCHAR pbIV, ULONG cbIV, PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptExportKey_t)(BCRYPT_KEY_HANDLE hKey, BCRYPT_KEY_HANDLE hExportKey, LPCWSTR pszBlobType, PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptImportKey_t)(BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_KEY_HANDLE hImportKey, LPCWSTR pszBlobType, BCRYPT_KEY_HANDLE *phKey, PUCHAR pbKeyObject, ULONG cbKeyObject, PUCHAR pbInput, ULONG cbInput, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptImportKeyPair_t)(BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_KEY_HANDLE hImportKey, LPCWSTR pszBlobType, BCRYPT_KEY_HANDLE *phKey, PUCHAR pbInput, ULONG cbInput, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptDuplicateKey_t)(BCRYPT_KEY_HANDLE hKey, BCRYPT_KEY_HANDLE *phNewKey, PUCHAR pbKeyObject, ULONG cbKeyObject, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptFinalizeKeyPair_t)(BCRYPT_KEY_HANDLE hKey, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptDestroyKey_t)(BCRYPT_KEY_HANDLE hKey);
typedef NTSTATUS WINAPI (* BCryptDestroySecret_t)(BCRYPT_SECRET_HANDLE hSecret);
typedef NTSTATUS WINAPI (* BCryptSignHash_t)(BCRYPT_KEY_HANDLE hKey, VOID *pPaddingInfo, PUCHAR pbInput, ULONG cbInput, PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptVerifySignature_t)(BCRYPT_KEY_HANDLE hKey, VOID *pPaddingInfo, PUCHAR pbHash, ULONG cbHash, PUCHAR pbSignature, ULONG cbSignature, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptSecretAgreement_t)(BCRYPT_KEY_HANDLE hPrivKey, BCRYPT_KEY_HANDLE hPubKey, BCRYPT_SECRET_HANDLE *phAgreedSecret, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptDeriveKey_t)(BCRYPT_SECRET_HANDLE hSharedSecret, LPCWSTR pwszKDF, BCryptBufferDesc *pParameterList, PUCHAR pbDerivedKey, ULONG cbDerivedKey, ULONG *pcbResult, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptCreateHash_t)(BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_HASH_HANDLE *phHash, PUCHAR pbHashObject, ULONG cbHashObject, PUCHAR pbSecret, ULONG cbSecret, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptHashData_t)(BCRYPT_HASH_HANDLE hHash, PUCHAR pbInput, ULONG cbInput, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptFinishHash_t)(BCRYPT_HASH_HANDLE hHash, PUCHAR pbOutput, ULONG cbOutput, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptDuplicateHash_t)(BCRYPT_HASH_HANDLE hHash, BCRYPT_HASH_HANDLE *phNewHash, PUCHAR pbHashObject, ULONG cbHashObject, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptDestroyHash_t)(BCRYPT_HASH_HANDLE hHash);
typedef NTSTATUS WINAPI (* BCryptGenRandom_t)(BCRYPT_ALG_HANDLE hAlgorithm, PUCHAR pbBuffer, ULONG cbBuffer, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptDeriveKeyCapi_t)(BCRYPT_HASH_HANDLE hHash, BCRYPT_ALG_HANDLE hTargetAlg, PUCHAR pbDerivedKey, ULONG cbDerivedKey, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptDeriveKeyPBKDF2_t)(BCRYPT_ALG_HANDLE hPrf, PUCHAR pbPassword, ULONG cbPassword, PUCHAR pbSalt, ULONG cbSalt, ULONGLONG cIterations, PUCHAR pbDerivedKey, ULONG cbDerivedKey, ULONG dwFlags);
typedef NTSTATUS WINAPI (* BCryptQueryProviderRegistration_t)(LPCWSTR pszProvider, ULONG dwMode, ULONG dwInterface, ULONG* pcbBuffer, PCRYPT_PROVIDER_REG *ppBuffer);
typedef NTSTATUS WINAPI (* BCryptEnumRegisteredProviders_t)(ULONG* pcbBuffer, PCRYPT_PROVIDERS *ppBuffer);
typedef NTSTATUS WINAPI (* BCryptCreateContext_t)(ULONG dwTable, LPCWSTR pszContext, PCRYPT_CONTEXT_CONFIG pConfig);
typedef NTSTATUS WINAPI (* BCryptDeleteContext_t)(ULONG dwTable, LPCWSTR pszContext);
typedef NTSTATUS WINAPI (* BCryptEnumContexts_t)(ULONG dwTable, ULONG* pcbBuffer, PCRYPT_CONTEXTS *ppBuffer);
typedef NTSTATUS WINAPI (* BCryptConfigureContext_t)(ULONG dwTable, LPCWSTR pszContext, PCRYPT_CONTEXT_CONFIG pConfig);
typedef NTSTATUS WINAPI (* BCryptQueryContextConfiguration_t)(ULONG dwTable, LPCWSTR pszContext, ULONG* pcbBuffer, PCRYPT_CONTEXT_CONFIG *ppBuffer);
typedef NTSTATUS WINAPI (* BCryptAddContextFunction_t)(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, ULONG dwPosition);
typedef NTSTATUS WINAPI (* BCryptRemoveContextFunction_t)(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction);
typedef NTSTATUS WINAPI (* BCryptEnumContextFunctions_t)(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, ULONG* pcbBuffer, PCRYPT_CONTEXT_FUNCTIONS *ppBuffer);
typedef NTSTATUS WINAPI (* BCryptConfigureContextFunction_t)(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, PCRYPT_CONTEXT_FUNCTION_CONFIG pConfig);
typedef NTSTATUS WINAPI (* BCryptQueryContextFunctionConfiguration_t)(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, ULONG* pcbBuffer, PCRYPT_CONTEXT_FUNCTION_CONFIG *ppBuffer);
typedef NTSTATUS WINAPI (* BCryptEnumContextFunctionProviders_t)(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, ULONG* pcbBuffer, PCRYPT_CONTEXT_FUNCTION_PROVIDERS *ppBuffer);
typedef NTSTATUS WINAPI (* BCryptSetContextFunctionProperty_t)(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, LPCWSTR pszProperty, ULONG cbValue, PUCHAR pbValue);
typedef NTSTATUS WINAPI (* BCryptQueryContextFunctionProperty_t)(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, LPCWSTR pszProperty, ULONG* pcbValue, PUCHAR *ppbValue);
typedef NTSTATUS WINAPI (* BCryptRegisterConfigChangeNotify_t)(HANDLE *phEvent);
typedef NTSTATUS WINAPI (* BCryptUnregisterConfigChangeNotify_t)(HANDLE hEvent);
typedef NTSTATUS WINAPI (* BCryptResolveProviders_t)(LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, LPCWSTR pszProvider, ULONG dwMode, ULONG dwFlags, ULONG* pcbBuffer, PCRYPT_PROVIDER_REFS *ppBuffer);
typedef NTSTATUS WINAPI (* BCryptGetFipsAlgorithmMode_t)(BOOLEAN *pfEnabled );


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Entry points
//

static BCryptOpenAlgorithmProvider_t                x_pfnBCryptOpenAlgorithmProvider;
static BCryptEnumAlgorithms_t                       x_pfnBCryptEnumAlgorithms;
static BCryptEnumProviders_t                        x_pfnBCryptEnumProviders;
static BCryptGetProperty_t                          x_pfnBCryptGetProperty;
static BCryptSetProperty_t                          x_pfnBCryptSetProperty;
static BCryptCloseAlgorithmProvider_t               x_pfnBCryptCloseAlgorithmProvider;
static BCryptFreeBuffer_t                           x_pfnBCryptFreeBuffer;
static BCryptGenerateSymmetricKey_t                 x_pfnBCryptGenerateSymmetricKey;
static BCryptGenerateKeyPair_t                      x_pfnBCryptGenerateKeyPair;
static BCryptEncrypt_t                              x_pfnBCryptEncrypt;
static BCryptDecrypt_t                              x_pfnBCryptDecrypt;
static BCryptExportKey_t                            x_pfnBCryptExportKey;
static BCryptImportKey_t                            x_pfnBCryptImportKey;
static BCryptImportKeyPair_t                        x_pfnBCryptImportKeyPair;
static BCryptDuplicateKey_t                         x_pfnBCryptDuplicateKey;
static BCryptFinalizeKeyPair_t                      x_pfnBCryptFinalizeKeyPair;
static BCryptDestroyKey_t                           x_pfnBCryptDestroyKey;
static BCryptDestroySecret_t                        x_pfnBCryptDestroySecret;
static BCryptSignHash_t                             x_pfnBCryptSignHash;
static BCryptVerifySignature_t                      x_pfnBCryptVerifySignature;
static BCryptSecretAgreement_t                      x_pfnBCryptSecretAgreement;
static BCryptDeriveKey_t                            x_pfnBCryptDeriveKey;
static BCryptCreateHash_t                           x_pfnBCryptCreateHash;
static BCryptHashData_t                             x_pfnBCryptHashData;
static BCryptFinishHash_t                           x_pfnBCryptFinishHash;
static BCryptDuplicateHash_t                        x_pfnBCryptDuplicateHash;
static BCryptDestroyHash_t                          x_pfnBCryptDestroyHash;
static BCryptGenRandom_t                            x_pfnBCryptGenRandom;
static BCryptDeriveKeyCapi_t                        x_pfnBCryptDeriveKeyCapi;
static BCryptDeriveKeyPBKDF2_t                      x_pfnBCryptDeriveKeyPBKDF2;
static BCryptQueryProviderRegistration_t            x_pfnBCryptQueryProviderRegistration;
static BCryptEnumRegisteredProviders_t              x_pfnBCryptEnumRegisteredProviders;
static BCryptCreateContext_t                        x_pfnBCryptCreateContext;
static BCryptDeleteContext_t                        x_pfnBCryptDeleteContext;
static BCryptEnumContexts_t                         x_pfnBCryptEnumContexts;
static BCryptConfigureContext_t                     x_pfnBCryptConfigureContext;
static BCryptQueryContextConfiguration_t            x_pfnBCryptQueryContextConfiguration;
static BCryptAddContextFunction_t                   x_pfnBCryptAddContextFunction;
static BCryptRemoveContextFunction_t                x_pfnBCryptRemoveContextFunction;
static BCryptEnumContextFunctions_t                 x_pfnBCryptEnumContextFunctions;
static BCryptConfigureContextFunction_t             x_pfnBCryptConfigureContextFunction;
static BCryptQueryContextFunctionConfiguration_t    x_pfnBCryptQueryContextFunctionConfiguration;
static BCryptEnumContextFunctionProviders_t         x_pfnBCryptEnumContextFunctionProviders;
static BCryptSetContextFunctionProperty_t           x_pfnBCryptSetContextFunctionProperty;
static BCryptQueryContextFunctionProperty_t         x_pfnBCryptQueryContextFunctionProperty;
static BCryptRegisterConfigChangeNotify_t           x_pfnBCryptRegisterConfigChangeNotify;
static BCryptUnregisterConfigChangeNotify_t         x_pfnBCryptUnregisterConfigChangeNotify;
static BCryptResolveProviders_t                     x_pfnBCryptResolveProviders;
static BCryptGetFipsAlgorithmMode_t                 x_pfnBCryptGetFipsAlgorithmMode;

static HMODULE                                      x_Crypt32dll;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Loader
//

#ifndef STATUS_NOT_SUPPORTED
#define STATUS_NOT_SUPPORTED 0xC00000BB
#endif

static void
load_error() {
    const DWORD rc = GetLastError();
    char  error[256] = {0};
    char  message[512] = {0};

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) error, sizeof(error)-1, NULL);

    _snprintf(message, sizeof(message),
        "Unable to load <crypt32.dll>\n%s (%ld)\n\nCrypto functionality disabled", (const char *)error, (long)rc);

    MessageBoxA(0, message, "Error", MB_OK|MB_ICONERROR);
}

#define RESOLVE_CRYPT32_API(__name) \
    if (0 == x_Crypt32dll) { \
        if (0 == (x_Crypt32dll = LoadLibraryA("crypt32.dll"))) { \
            x_Crypt32dll = (HMODULE)-1; \
            load_error(); \
        } \
    } \
    if ((HMODULE)-1 != x_Crypt32dll) x_pfn##__name = (__name##_t)GetProcAddress(x_Crypt32dll, #__name); \
    if (0 == x_pfn##__name) return STATUS_NOT_SUPPORTED; \
    return (x_pfn##__name)

#define RESOLVE_CRYPT32_API_VOID(__name) \
    if (0 == x_Crypt32dll) { \
        if (0 == (x_Crypt32dll = LoadLibraryA("crypt32.dll"))) { \
            x_Crypt32dll = (HMODULE)-1; \
        } \
    } \
    if ((HMODULE)-1 != x_Crypt32dll) x_pfn##__name = (__name##_t)GetProcAddress(x_Crypt32dll, #__name); \
    if (0 == x_pfn##__name) return; \
    (x_pfn##__name)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Public API
//

NTSTATUS WINAPI BCryptOpenAlgorithmProvider(BCRYPT_ALG_HANDLE *phAlgorithm, LPCWSTR pszAlgId, LPCWSTR pszImplementation, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptOpenAlgorithmProvider)
        (phAlgorithm, pszAlgId, pszImplementation, dwFlags);
}

NTSTATUS WINAPI BCryptEnumAlgorithms(ULONG dwAlgOperations, ULONG *pAlgCount, BCRYPT_ALGORITHM_IDENTIFIER **ppAlgList, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptEnumAlgorithms)
        (dwAlgOperations, pAlgCount, ppAlgList, dwFlags);
}

NTSTATUS WINAPI BCryptEnumProviders(LPCWSTR pszAlgId, ULONG *pImplCount, BCRYPT_PROVIDER_NAME **ppImplList, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptEnumProviders)
        (pszAlgId, pImplCount, ppImplList, dwFlags);
}

NTSTATUS WINAPI BCryptGetProperty(BCRYPT_HANDLE hObject, LPCWSTR pszProperty, PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptGetProperty)
        (hObject, pszProperty, pbOutput, cbOutput, pcbResult, dwFlags);
}

NTSTATUS WINAPI BCryptSetProperty(BCRYPT_HANDLE hObject, LPCWSTR pszProperty, PUCHAR pbInput, ULONG cbInput, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptSetProperty)
        (hObject, pszProperty, pbInput, cbInput, dwFlags);
}

NTSTATUS WINAPI BCryptCloseAlgorithmProvider(BCRYPT_ALG_HANDLE hAlgorithm, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptCloseAlgorithmProvider)
        (hAlgorithm, dwFlags);
}

VOID    WINAPI BCryptFreeBuffer(PVOID pvBuffer) {
    RESOLVE_CRYPT32_API_VOID(BCryptFreeBuffer);
        (pvBuffer);
}

NTSTATUS WINAPI BCryptGenerateSymmetricKey(BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_KEY_HANDLE *phKey, PUCHAR pbKeyObject, ULONG cbKeyObject, PUCHAR pbSecret, ULONG cbSecret, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptGenerateSymmetricKey)
        (hAlgorithm, phKey, pbKeyObject, cbKeyObject, pbSecret, cbSecret, dwFlags);
}

NTSTATUS WINAPI BCryptGenerateKeyPair(BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_KEY_HANDLE *phKey, ULONG dwLength, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptGenerateKeyPair)
        (hAlgorithm, phKey, dwLength, dwFlags);
}

NTSTATUS WINAPI BCryptEncrypt(BCRYPT_KEY_HANDLE hKey, PUCHAR pbInput, ULONG cbInput, VOID *pPaddingInfo, PUCHAR pbIV, ULONG cbIV, PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptEncrypt)
        (hKey, pbInput, cbInput, pPaddingInfo, pbIV, cbIV, pbOutput, cbOutput, pcbResult, dwFlags);
}

NTSTATUS WINAPI BCryptDecrypt(BCRYPT_KEY_HANDLE hKey, PUCHAR pbInput, ULONG cbInput, VOID *pPaddingInfo, PUCHAR pbIV, ULONG cbIV, PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptDecrypt)
        (hKey, pbInput, cbInput, pPaddingInfo, pbIV, cbIV, pbOutput, cbOutput, pcbResult, dwFlags);
}

NTSTATUS WINAPI BCryptExportKey(BCRYPT_KEY_HANDLE hKey, BCRYPT_KEY_HANDLE hExportKey, LPCWSTR pszBlobType, PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptExportKey)
        (hKey, hExportKey, pszBlobType, pbOutput, cbOutput, pcbResult, dwFlags);
}

NTSTATUS WINAPI BCryptImportKey(BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_KEY_HANDLE hImportKey, LPCWSTR pszBlobType, BCRYPT_KEY_HANDLE *phKey, PUCHAR pbKeyObject, ULONG cbKeyObject, PUCHAR pbInput, ULONG cbInput, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptImportKey)
        (hAlgorithm, hImportKey, pszBlobType, phKey, pbKeyObject, cbKeyObject, pbInput, cbInput, dwFlags);
}

NTSTATUS WINAPI BCryptImportKeyPair(BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_KEY_HANDLE hImportKey, LPCWSTR pszBlobType, BCRYPT_KEY_HANDLE *phKey, PUCHAR pbInput, ULONG cbInput, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptImportKeyPair)
        (hAlgorithm, hImportKey, pszBlobType, phKey, pbInput, cbInput, dwFlags);
}

NTSTATUS WINAPI BCryptDuplicateKey(BCRYPT_KEY_HANDLE hKey, BCRYPT_KEY_HANDLE *phNewKey, PUCHAR pbKeyObject, ULONG cbKeyObject, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptDuplicateKey)
        (hKey, phNewKey, pbKeyObject, cbKeyObject, dwFlags);
}

NTSTATUS WINAPI BCryptFinalizeKeyPair(BCRYPT_KEY_HANDLE hKey, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptFinalizeKeyPair)
        (hKey, dwFlags);
}

NTSTATUS WINAPI BCryptDestroyKey(BCRYPT_KEY_HANDLE hKey) {
    RESOLVE_CRYPT32_API(BCryptDestroyKey)
        (hKey);
}

NTSTATUS WINAPI BCryptDestroySecret(BCRYPT_SECRET_HANDLE hSecret) {
    RESOLVE_CRYPT32_API(BCryptDestroySecret)
        (hSecret);
}

NTSTATUS WINAPI BCryptSignHash(BCRYPT_KEY_HANDLE hKey, VOID *pPaddingInfo, PUCHAR pbInput, ULONG cbInput, PUCHAR pbOutput, ULONG cbOutput, ULONG *pcbResult, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptSignHash)
        (hKey, pPaddingInfo, pbInput, cbInput, pbOutput, cbOutput, pcbResult, dwFlags);
}

NTSTATUS WINAPI BCryptVerifySignature(BCRYPT_KEY_HANDLE hKey, VOID *pPaddingInfo, PUCHAR pbHash, ULONG cbHash, PUCHAR pbSignature, ULONG cbSignature, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptVerifySignature)
        (hKey, pPaddingInfo, pbHash, cbHash, pbSignature, cbSignature, dwFlags);
}

NTSTATUS WINAPI BCryptSecretAgreement(BCRYPT_KEY_HANDLE hPrivKey, BCRYPT_KEY_HANDLE hPubKey, BCRYPT_SECRET_HANDLE *phAgreedSecret, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptSecretAgreement)
        (hPrivKey, hPubKey, phAgreedSecret, dwFlags);
}

NTSTATUS WINAPI BCryptDeriveKey(BCRYPT_SECRET_HANDLE hSharedSecret, LPCWSTR pwszKDF, BCryptBufferDesc *pParameterList, PUCHAR pbDerivedKey, ULONG cbDerivedKey, ULONG *pcbResult, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptDeriveKey)
        (hSharedSecret, pwszKDF, pParameterList, pbDerivedKey, cbDerivedKey, pcbResult, dwFlags);
}

NTSTATUS WINAPI BCryptCreateHash(BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_HASH_HANDLE *phHash, PUCHAR pbHashObject, ULONG cbHashObject, PUCHAR pbSecret, ULONG cbSecret, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptCreateHash)
        (hAlgorithm, phHash, pbHashObject, cbHashObject, pbSecret, cbSecret, dwFlags);
}

NTSTATUS WINAPI BCryptHashData(BCRYPT_HASH_HANDLE hHash, PUCHAR pbInput, ULONG cbInput, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptHashData)
        (hHash, pbInput, cbInput, dwFlags);
}

NTSTATUS WINAPI BCryptFinishHash(BCRYPT_HASH_HANDLE hHash, PUCHAR pbOutput, ULONG cbOutput, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptFinishHash)
        (hHash, pbOutput, cbOutput, dwFlags);
}

NTSTATUS WINAPI BCryptDuplicateHash(BCRYPT_HASH_HANDLE hHash, BCRYPT_HASH_HANDLE *phNewHash, PUCHAR pbHashObject, ULONG cbHashObject, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptDuplicateHash)
        (hHash, phNewHash, pbHashObject, cbHashObject, dwFlags);
}

NTSTATUS WINAPI BCryptDestroyHash(BCRYPT_HASH_HANDLE hHash) {
    RESOLVE_CRYPT32_API(BCryptDestroyHash)
        (hHash);
}

NTSTATUS WINAPI BCryptGenRandom(BCRYPT_ALG_HANDLE hAlgorithm, PUCHAR pbBuffer, ULONG cbBuffer, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptGenRandom)
        (hAlgorithm, pbBuffer, cbBuffer, dwFlags);
}

NTSTATUS WINAPI BCryptDeriveKeyCapi(BCRYPT_HASH_HANDLE hHash, BCRYPT_ALG_HANDLE hTargetAlg, PUCHAR pbDerivedKey, ULONG cbDerivedKey, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptDeriveKeyCapi)
        (hHash, hTargetAlg, pbDerivedKey, cbDerivedKey, dwFlags);
}

NTSTATUS WINAPI BCryptDeriveKeyPBKDF2(BCRYPT_ALG_HANDLE hPrf, PUCHAR pbPassword, ULONG cbPassword, PUCHAR pbSalt, ULONG cbSalt, ULONGLONG cIterations, PUCHAR pbDerivedKey, ULONG cbDerivedKey, ULONG dwFlags) {
    RESOLVE_CRYPT32_API(BCryptDeriveKeyPBKDF2)
        (hPrf, pbPassword, cbPassword, pbSalt, cbSalt, cIterations, pbDerivedKey, cbDerivedKey, dwFlags);
}

NTSTATUS WINAPI BCryptQueryProviderRegistration(LPCWSTR pszProvider, ULONG dwMode, ULONG dwInterface, ULONG* pcbBuffer, PCRYPT_PROVIDER_REG *ppBuffer) {
    RESOLVE_CRYPT32_API(BCryptQueryProviderRegistration)
        (pszProvider, dwMode, dwInterface, pcbBuffer, ppBuffer);
}

NTSTATUS WINAPI BCryptEnumRegisteredProviders(ULONG* pcbBuffer, PCRYPT_PROVIDERS *ppBuffer) {
    RESOLVE_CRYPT32_API(BCryptEnumRegisteredProviders)
        (pcbBuffer, ppBuffer);
}

NTSTATUS WINAPI BCryptCreateContext(ULONG dwTable, LPCWSTR pszContext, PCRYPT_CONTEXT_CONFIG pConfig) {
    RESOLVE_CRYPT32_API(BCryptCreateContext)
        (dwTable, pszContext, pConfig);
}

NTSTATUS WINAPI BCryptDeleteContext(ULONG dwTable, LPCWSTR pszContext) {
    RESOLVE_CRYPT32_API(BCryptDeleteContext)
        (dwTable, pszContext);
}

NTSTATUS WINAPI BCryptEnumContexts(ULONG dwTable, ULONG* pcbBuffer, PCRYPT_CONTEXTS *ppBuffer) {
    RESOLVE_CRYPT32_API(BCryptEnumContexts)
        (dwTable, pcbBuffer, ppBuffer);
}

NTSTATUS WINAPI BCryptConfigureContext(ULONG dwTable, LPCWSTR pszContext, PCRYPT_CONTEXT_CONFIG pConfig) {
    RESOLVE_CRYPT32_API(BCryptConfigureContext)
        (dwTable, pszContext, pConfig);
}

NTSTATUS WINAPI BCryptQueryContextConfiguration(ULONG dwTable, LPCWSTR pszContext, ULONG* pcbBuffer, PCRYPT_CONTEXT_CONFIG *ppBuffer) {
    RESOLVE_CRYPT32_API(BCryptQueryContextConfiguration)
        (dwTable, pszContext, pcbBuffer, ppBuffer);
}

NTSTATUS WINAPI BCryptAddContextFunction(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, ULONG dwPosition) {
    RESOLVE_CRYPT32_API(BCryptAddContextFunction)
        (dwTable, pszContext, dwInterface, pszFunction, dwPosition);
}

NTSTATUS WINAPI BCryptRemoveContextFunction(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction) {
    RESOLVE_CRYPT32_API(BCryptRemoveContextFunction)
        (dwTable, pszContext, dwInterface, pszFunction);
}

NTSTATUS WINAPI BCryptEnumContextFunctions(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, ULONG* pcbBuffer, PCRYPT_CONTEXT_FUNCTIONS *ppBuffer) {
    RESOLVE_CRYPT32_API(BCryptEnumContextFunctions)
        (dwTable, pszContext, dwInterface, pcbBuffer, ppBuffer);
}

NTSTATUS WINAPI BCryptConfigureContextFunction(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, PCRYPT_CONTEXT_FUNCTION_CONFIG pConfig) {
    RESOLVE_CRYPT32_API(BCryptConfigureContextFunction)
        (dwTable, pszContext, dwInterface, pszFunction, pConfig);
}

NTSTATUS WINAPI BCryptQueryContextFunctionConfiguration(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, ULONG* pcbBuffer, PCRYPT_CONTEXT_FUNCTION_CONFIG *ppBuffer) {
    RESOLVE_CRYPT32_API(BCryptQueryContextFunctionConfiguration)
        (dwTable, pszContext, dwInterface, pszFunction, pcbBuffer, ppBuffer);
}

NTSTATUS WINAPI BCryptEnumContextFunctionProviders(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, ULONG* pcbBuffer, PCRYPT_CONTEXT_FUNCTION_PROVIDERS *ppBuffer) {
    RESOLVE_CRYPT32_API(BCryptEnumContextFunctionProviders)
        (dwTable, pszContext, dwInterface, pszFunction, pcbBuffer, ppBuffer);
}

NTSTATUS WINAPI BCryptSetContextFunctionProperty(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, LPCWSTR pszProperty, ULONG cbValue, PUCHAR pbValue) {
    RESOLVE_CRYPT32_API(BCryptSetContextFunctionProperty)
        (dwTable, pszContext, dwInterface, pszFunction, pszProperty, cbValue, pbValue);
}

NTSTATUS WINAPI BCryptQueryContextFunctionProperty(ULONG dwTable, LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, LPCWSTR pszProperty, ULONG* pcbValue, PUCHAR *ppbValue) {
    RESOLVE_CRYPT32_API(BCryptQueryContextFunctionProperty)
        (dwTable, pszContext, dwInterface, pszFunction, pszProperty, pcbValue, ppbValue);
}

NTSTATUS WINAPI BCryptRegisterConfigChangeNotify(HANDLE *phEvent) {
    RESOLVE_CRYPT32_API(BCryptRegisterConfigChangeNotify)
        (phEvent);
}

NTSTATUS WINAPI BCryptUnregisterConfigChangeNotify(HANDLE hEvent) {
    RESOLVE_CRYPT32_API(BCryptUnregisterConfigChangeNotify)
        (hEvent);
}

NTSTATUS WINAPI BCryptResolveProviders(LPCWSTR pszContext, ULONG dwInterface, LPCWSTR pszFunction, LPCWSTR pszProvider, ULONG dwMode, ULONG dwFlags, ULONG* pcbBuffer, PCRYPT_PROVIDER_REFS *ppBuffer) {
    RESOLVE_CRYPT32_API(BCryptResolveProviders)
        (pszContext, dwInterface, pszFunction, pszProvider, dwMode, dwFlags, pcbBuffer, ppBuffer);
}

NTSTATUS WINAPI BCryptGetFipsAlgorithmMode(BOOLEAN *pfEnabled ) {
    RESOLVE_CRYPT32_API(BCryptGetFipsAlgorithmMode)
        (pfEnabled);
}

/*end*/


