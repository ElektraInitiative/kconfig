# Differences between Elektra config and .ini config

## Testing

Most of the tests check if the .ini backend specific files exist. 
These files are not created by the Elektra backend and the tests fail.

This is circumvented by ignoring the file checks, if KConfig has been linked against Elektra.

Not all tests have been patched yet.

 