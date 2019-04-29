BLE persistent storage (flash) for bonded devices {#ble_pers_strg}
=================================================

## Overview

BLE Manager is able to automatically manage list of bonded devices in both RAM and on persistent
storage, i.e. QSPI flash. By default, devices are managed in RAM only and persistent storage has
to be explicitly enabled in application configuration. Device data is stored using NVMS on generic
partition.

Two kind of data are stored:
- device pairing data (exchanged keys and related information)
- application-defined data managed using ble_storage API (only values set with `persistent` flag
  are stored in flash, e.g. CCC descriptor values)

## Configuration

Persistent storage can be enabled in application by following entries in application configuration
file:

~~~{.c}
    // enable BLE persistent storage
    #define CONFIG_BLE_STORAGE

    // enable Flash and NVMS adapters with VES (required by BLE persistent storage)
    #define dg_configFLASH_ADAPTER                  1
    #define dg_configNVMS_ADAPTER                   1
    #define dg_configNVMS_VES                       1
~~~

By default, data are stored on partition at following offsets:
- device pairing data at offset 0x0000 (maximum length depends on max number of bonded devices)
- application data at offset 0x0500 (maximum length of 1024 bytes)

This can be changed using following entries in application configuration file:

~~~{.c}
    // set pairing data storage at offset 0x1000 on generic partition
    #define CONFIG_BLE_STORAGE_KEY_PART_OFFSET (0x1000)
    // set application data storage at offset 0x1000 on generic partition
    #define CONFIG_BLE_STORAGE_APV_PART_OFFSET (0x1500)
    // set maximum length of application data storage to 512
    #define CONFIG_BLE_STORAGE_APV_PART_LENGTH (512)
~~~

In case application tries to write more application data than allowed, data that do not fit
will be discarded.

A compile-time assert is enabled to ensure gap between device data offset and application data
offset is large enough to fit maximum number of bonded devices, as configured.

Number of maximum bonded devices can be set in ble_config.h file:

~~~{.c}
    // by default 8 devices can be bonded at the same time
    #define defaultBLE_MAX_BONDED (8)
~~~

When trying to bond more devices than allowed an error will be returned to application and it
should be handled there depending on requirements, i.e. application can either unbond one of
devices or perform pairing without bonding.

## Storage format (device data)

### Header
Device storage starts with magic value which identifies data region - if this values does not
match with value included in software, data will not be loaded during startup as it is assumed
they are not valid (i.e. were never written). Magic value is as follows:

~~~{.c}
    42 4C 45 5F 4B 45 59 00
~~~

In ASCII code this translates to `BLE_KEY\x00`. Last byte (0x00) is used for storage format
versioning and should be changed whenever storage layout changes. This ensures that data written
in old storage layout will not be loaded into new structure.

Magic value is followed immediately by single byte value which contains number of entries for
device data. When loading data from storage this value will be compared against current value of
`defaultBLE_MAX_BONDED` and lesser of both will determine maximum number of device entries to be
read from flash. On first write, this will be updated with `defaultBLE_MAX_BONDED`. This is to
ensure backwards-compatibility in case `defaultBLE_MAX_BONDED` is changed between software versions
without need to change magic value.

### Device data
Header is followed by actual bonding information for each device. Number of entries is always
the same as stored in header (see above). Each entry has format as defined below. Note that
there are unused bytes in structure below which are used for padding and have undefined value.

| offset   | len | contents
|--------- | --- | -------------
| 0x0000   | 4   | flags
| 0x0004   | 7   | device address
| 0x0010   | 27  | LTK
| 0x0030   | 27  | remote LTK
| 0x0050   | 16  | IRK
| 0x0060   | 18  | CSRK
| 0x0074   | 18  | remote CSRK

##### Flags
Flags value is a bitmask which determines other device properties, in particular it defines which
fields in device entry have valid value.

    0x0001 = device entry is not used (i.e. any other data for this entry is ignored)
    0x0002 = device entry has valid LTK
    0x0004 = device entry has valid remote LTK
    0x0008 = device entry has valid IRK
    0x0010 = device entry has valid CSRK
    0x0020 = device entry has valid remote CSRK
    0x0040 = valid keys for this device entry are authenticated (MITM bonding was used)

##### Device address

| offset | len |  contents
|------- | --- | ------------------------------------------
| 0x0000 | 1   | address type (0x00 = public, 0x01 = random)
| 0x0001 | 6   | device address

##### LTK and remote LTK

| offset | len | contents
|------- | --- | -----------
| 0x0000 |  8  | Rand
| 0x0008 |  2  | EDIV
| 0x000A | 16  | LTK
| 0x001A |  1  | key size

##### IRK

| offset | len | contents
|------- | --- | ----------
|0x0000  | 16  |  IRK

##### CSRK and remote CSRK

| offset | len | contents
|------- | --- | -------------
| 0x0000 | 16  | CSRK
| 0x0010 |  2  | sign counter


## Storage format (application data)

### Header
Application storage starts with magic value which identifies data region - if this values does not
match with value included in software, data will not be loaded during startup as it is assumed they
are not valid (i.e. were never written). Magic value is as follows:

~~~{.c}
    42 4C 45 5F 41 50 56 01
~~~

In ASCII code this translates to `BLE_APV\x01`. The last byte (0x01) is used for storage format
versioning and should be changed whenever the storage layout changes. This ensures that data
written in old storage layout will not be loaded into new structure.

### Application data
Header is followed by the actual application data for each device. Data is a sequence of triplets
in form of <type, key, value>. Data for each device starts with `type=0x01` element and follows
until next `type=0x01` or `type=0x00` element is found.

##### Type

| type | len | contents
|----- | --- | ----------------------------------------------------------------------------
| 0x00 | 0   | empty element, used to indicate end of data
| 0x01 | 7   | device address
| 0x02 | 4   | integer value
| 0x03 | var | variable length data

##### Key

Key is 4-byte integer value corresponding to key used in ble_storage API calls.

##### Device address

| offset | len |  contents
|------- | --- | -------------------------------------------
| 0x0000 | 1   | address type (0x00 = public, 0x01 = random)
| 0x0001 | 6   | device address

##### Variable length data

| offset | len |  contents
|------- | --- | -------------------------------------------
| 0x0000 | 2   | length of following data
| 0x0002 | var | application data value, number of bytes is `len`
