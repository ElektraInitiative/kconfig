
## Configure Elektra Backend

The Elektra backend can be configured in multiple ways.  Elektra recommends to store the configuration of an application at a path which contains the distributor, the name of the application, the major version as well as a profile. For applications using KConfig the following path is used:

```
/sw/org/kde/<app_name>/#<major_version>/<profile>
```

If no value is supplied for `major_version` version 0 is assumed if the KConfig object was created using the application info. Else the current major version of the KDE Frameworks is assumed. (Which is 5 at the time of writing.)

`profile` has a default value of "current" and can be set separately for application configuration and global configuration. The settings for the global configuration can only be changed using environment variables or command line parameters. The profile of the application configuration can also be set programmatically.

If the configuration object is created using `KConfig(QString)` and the passed string is no absolute path, the passed string is assumed to be the application name. Should the passed string be an absolute path a KConfig object with the `INI` backend is created. If present the `app_name` is taken from an instance of `QCoreApplication`. If no  `app_name` is found, creating a configuration object will fail.

### Command Line Parameters & Environment Variables

If setting the parameters for Elektra is not possible programmatically, values provided from the command line or environment variables take precedence over default parameters. Using parameters from the command line is only possible, if the application is a `QApplication`.

The table below shows how to set Elektra parameters for the application configuration.

| Parameter       | Command Line Parameter        | Environment Variable        |
| --------------- | ----------------------------- | --------------------------- |
| `app_name`      | `--kconfig-app-name`          | `KCONFIG_APP_NAME`          |
| `major_version` | `--kconfig-app-major-version` | `KCONFIG_APP_MAJOR_VERSION` |
| `profile`       | `--kconfig-app-profile`       | `KCONFIG_APP_PROFILE`       |

For the global configuration only the profile can be overwritten. If a profile for the application configuration has been supplied, but no global overwrite exists, the same profile is used for the application and the globals configuration.

| Parameter | Command Line Parameter      | Environment Variable      | Fallback Value (from app config) |
| --------- | --------------------------- | ------------------------- | -------------------------------- |
| `profile` | `--elektra-globals-profile` | `KCONFIG_GLOBALS_PROFILE` | `profile`                        |

