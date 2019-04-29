Secure Boot Loader {#secure_boot}
==================

## Overview

This application is an alternative bootloader which could be used as second stage bootloader during
Software Update over the Air (SUOTA) procedure.

Features:

- Checking device integrity:
  - Comparing bootloader CRC placed in OTP header with calculated for this bootlaoder
  - Checking 'Secure Device' field in OTP header - some functionalities are available only for
    secured devices
  - Validation of the symmetric keys stored in OTP used in encryption/decryption
  - Validation of the root/public keys stored in OTP used in image signature validation
  - Checking minimum FW version array stored in the OTP
- Firmware validation (update and current application images):
  - Checking SUOTA 1.1 header
  - Checking image's CRC
  - Validation of the header security extension content
  - Checking FW version number with current minimum FW version
- Copying FW stored on the 'update' partion to 'executable' partition
- Root/public keys revocation possibility
- Upgrading minimum FW version array
- Customizable code (hooks)

This application could be stored in the OTP (default place). Proper build configuration of the
'ble_suota_loader' project must be used if above features should be available. Each configuration
with '_Secure' suffix builds Secure Boot Loader. This bootloader doesn't use FreeRTOS and BLE. The
SUOTA procedure must be handled by firmware - application image e.g. PXP Reporter.

## Installation procedure

Secure bootloader is available only for DA14683 devices - there is no support for DA14681.

1. Build firmware image (application with SUOTA support) - for example PXP Reporter with SUOTA support.
2. Build secure bootloader - target DA14683-00-Release_OTP_Secure depends on used device.
3. Call secure_image_config.py Python script. It creates secure config XML file and XML file with keys.
   Answering on few questions will be required during script execution.
4. Call initial_flash.py Python script with '--secure_config' flag and path to the secure config XML
   file, '--keys' flag and path to the XML file with keys (symmetric and asymmetric) and the
   application binary. Secondary bootloader binary (Secure Boot Loader) which has been built in
   previous step will be used by defaults - this location could be overwritten by '--bootloader' flag.

Both scripts are using **Python 3**.

**Warning: initial_flash.py script performs writing to the One Time Programmable memory. When this**
**procedure is called with invalid configuration/firmware/bootloader files then the device**
**may become unusable!**
