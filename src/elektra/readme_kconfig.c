keyNew("system/elektra/modules/kconfig/infos",
KEY_VALUE, "Information about the kconfig plugin is in keys below", KEY_END),
keyNew("system/elektra/modules/kconfig/infos/author",
KEY_VALUE, "Author Name <elektra@libelektra.org>", KEY_END),
keyNew("system/elektra/modules/kconfig/infos/licence",
KEY_VALUE, "BSD", KEY_END),
keyNew("system/elektra/modules/kconfig/infos/needs",
KEY_VALUE, "base64 directoryvalue", KEY_END),
keyNew("system/elektra/modules/kconfig/infos/provides",
KEY_VALUE, "storage/kconfig storage kconfig", KEY_END),
keyNew("system/elektra/modules/kconfig/infos/recommends",
KEY_VALUE, "", KEY_END),
keyNew("system/elektra/modules/kconfig/infos/placements",
KEY_VALUE, "getstorage setstorage", KEY_END),
keyNew("system/elektra/modules/kconfig/infos/status",
KEY_VALUE, "maintained preview unfinished concept", KEY_END),
keyNew("system/elektra/modules/kconfig/infos/metadata",
KEY_VALUE, "", KEY_END),
keyNew("system/elektra/modules/kconfig/infos/description",
KEY_VALUE, "one-line description of kconfig\n"
"\n"
"## Introduction\n"
"\n"
"Copy this kconfig if you want to start a new\n"
"plugin written in C++.\n"
"\n"
"## Usage\n"
"\n"
"You can use `scripts/copy-kconfig`\n"
"to automatically rename everything to your\n"
"plugin name:\n"
"\n"
"```bash\n"
"cd src/plugins\n"
"../../scripts/copy-kconfig yourplugin\n"
"```\n"
"\n"
"Then update the README.md of your newly created plugin:\n"
"\n"
"- enter your full name+email in `infos/author`\n"
"- make sure `status`, `placements`, and other clauses conform to\n"
"  descriptions in `doc/CONTRACT.ini`\n"
"- update the one-line description above\n"
"- add your plugin in `src/plugins/README.md`\n"
"- and rewrite the rest of this `README.md` to give a great\n"
"  explanation of what your plugin does\n"
"\n"
"## Dependencies\n"
"\n"
"None.\n"
"\n"
"## Examples\n"
"\n"
"```sh\n"
"# Backup-and-Restore: user/tests/kconfig\n"
"\n"
"kdb set user/tests/kconfig/key value\n"
"#> Create a new key user/tests/kconfig/key with string \"value\"\n"
"\n"
"kdb get /tests/kconfig/key\n"
"#> value\n"
"```\n"
"\n"
"## Limitations\n"
"\n"
"None.", KEY_END),
