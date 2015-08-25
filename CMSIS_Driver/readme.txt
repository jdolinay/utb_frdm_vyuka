Ovladace podle standardu CMSIS
zalozeno na implementaci Keil pro LPC18xx

[.] - sovladace pro jednotlive MCU

/arm - Pouze pro informaci. Neni potrebne pro preklad! Soubory poskytovane ARM,
        tj. "generic" hlavickove soubory a templates pro drivery.

/Config - obsahuje .h soubor s konfiguraci ovladacu, napr. piny, ktere ovladac pouziva.




Dalsi informace
----------------
Usporadani slozek podle ARM je asi takoveto:
CMSIS/Driver/Include - .h soubory obecne pro danou "tridu" driveru, napr. ovladac pro UART, I2C, priklad: Driver_I2C.h
CMSIS/Pack/ - "balicky" s implementaci podle vyrobce nebo MCU?
  CMSIS/Pack/[vyrobce_nebo_mcu]/CMSIS_Driver - ovladace daneho (vyrobce nebo MCU)?
  CMSIS/Pack/[vyrobce_nebo_mcu]/CMSIS_Driver/Config - RTE_device.h - konfigurace pro tyto drivery.


Zjednodusene usporadani pro utb_frdm_vyuka:

CMSIS_Driver - Obsahuje i "obecne" ARM soubory .h pro drivery i konkretni implementaci.
CMSIS_Driver/Config - obsahuje konfiguracni soubor RTE_Device.h