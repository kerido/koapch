#pragma once

#include <aclapi.h>

#define SD_SIZE (2048 + SECURITY_DESCRIPTOR_MIN_LENGTH)

class RegKeyPermissionModifier
{
public:
   BOOL AddAccessRights(HKEY hKey, PSID pSID, DWORD dwAcessMask)
   {
      //  SD variables.

      UCHAR          ucSDbuf[SD_SIZE];
      PSECURITY_DESCRIPTOR pSD=(PSECURITY_DESCRIPTOR)ucSDbuf;
      DWORD          dwSDLengthNeeded   =   SD_SIZE;

      // ACL variables.

      PACL           pACL;
      BOOL           bDaclPresent;
      BOOL           bDaclDefaulted;
      ACL_SIZE_INFORMATION AclInfo;

      // New ACL variables.

      PACL           pNewACL;
      DWORD          dwNewACLSize;

      // New SD variables.

      UCHAR                NewSD[SECURITY_DESCRIPTOR_MIN_LENGTH];
      PSECURITY_DESCRIPTOR psdNewSD=(PSECURITY_DESCRIPTOR)NewSD;

      // Temporary ACE.

      PVOID          pTempAce;
      UINT           CurrentAceIndex;

      // STEP 2: Get SID (parameter).

      // STEP 3: Get security descriptor (SD) for key.

      if(ERROR_SUCCESS!=RegGetKeySecurity(hKey,
                    (SECURITY_INFORMATION)(DACL_SECURITY_INFORMATION),
                    pSD,
                    &dwSDLengthNeeded))
      {
         //printf("Error %d:RegGetKeySecurity\n",GetLastError());
         return(FALSE);
      }

      // STEP 4: Initialize new SD.

      if(!InitializeSecurityDescriptor
         (psdNewSD,SECURITY_DESCRIPTOR_REVISION))
      {
         //printf("Error %d:InitializeSecurityDescriptor\n",GetLastError());
         return(FALSE);
      }

      // STEP 5: Get DACL from SD.

      if (!GetSecurityDescriptorDacl(pSD,
                       &bDaclPresent,
                       &pACL,
                       &bDaclDefaulted))
      {
         //printf("Error %d:GetSecurityDescriptorDacl\n",GetLastError());
         return(FALSE);
      }

      // STEP 6: Get key ACL size information.

      if(!GetAclInformation(pACL,&AclInfo,sizeof(ACL_SIZE_INFORMATION),
         AclSizeInformation))
      {
         //printf("Error %d:GetAclInformation\n",GetLastError());
         return(FALSE);
      }

      // STEP 7: Compute size needed for the new ACL.

      dwNewACLSize = AclInfo.AclBytesInUse +
                     sizeof(ACCESS_ALLOWED_ACE) +
                     GetLengthSid(pSID) - sizeof(DWORD);

      // STEP 8: Allocate memory for new ACL.

      pNewACL = (PACL)LocalAlloc(LPTR, dwNewACLSize);

      // STEP 9: Initialize the new ACL.

      if(!InitializeAcl(pNewACL, dwNewACLSize, ACL_REVISION2))
      {
         //printf("Error %d:InitializeAcl\n",GetLastError());
         LocalFree((HLOCAL) pNewACL);
         return(FALSE);
      }

      // STEP 14: Add the access-allowed ACE to the new DACL.

      if(!AddAccessAllowedAceEx(pNewACL,ACL_REVISION,CONTAINER_INHERIT_ACE,dwAcessMask, pSID))
      {
         //printf("Error %d:AddAccessAllowedAce",GetLastError());
         LocalFree((HLOCAL) pNewACL);
         return(FALSE);
      }

      // STEP 10: If DACL is present, copy it to a new DACL.

      if(bDaclPresent)  // Only copy if DACL was present.
      {
         // STEP 11: Copy the file's ACEs to our new ACL.

         if(AclInfo.AceCount)
         {

            for(CurrentAceIndex = 0; CurrentAceIndex < AclInfo.AceCount;
               CurrentAceIndex++)
            {
               // STEP 12: Get an ACE.

               if(!GetAce(pACL,CurrentAceIndex,&pTempAce))
               {
                 //printf("Error %d: GetAce\n",GetLastError());
                 LocalFree((HLOCAL) pNewACL);
                 return(FALSE);
               }

                // STEP 13: Add the ACE to the new ACL.

               if(!AddAce(pNewACL, ACL_REVISION, MAXDWORD, pTempAce,
                  ((PACE_HEADER)pTempAce)->AceSize))
               {
                  //printf("Error %d:AddAce\n",GetLastError());
                  LocalFree((HLOCAL) pNewACL);
                  return(FALSE);
               }

             }
         }
      }

      // STEP 15: Set our new DACL to the file SD.

      if (!SetSecurityDescriptorDacl(psdNewSD,
                        TRUE,
                        pNewACL,
                        FALSE))
      {
         //printf("Error %d:SetSecurityDescriptorDacl",GetLastError());
         LocalFree((HLOCAL) pNewACL);
         return(FALSE);
      }

      // STEP 16: Set the SD to the key.

      if (ERROR_SUCCESS!=RegSetKeySecurity(hKey, DACL_SECURITY_INFORMATION,psdNewSD))
      {
         //printf("Error %d:RegSetKeySecurity\n",GetLastError());
         LocalFree((HLOCAL) pNewACL);
         return(FALSE);
      }

      // STEP 17: Free the memory allocated for the new ACL.

      LocalFree((HLOCAL) pNewACL);
      return(TRUE);
   }
};