# dircmp

### The simplest directory comparison command line tool

```
> dircmp "example/a" "example/b"

|- <root>
|  |- file "abc.txt" is missing in "example/a"
|  |- file "def.txt" is missing in "example/b"
|  |- file "test.txt" differs
|  |- file "baz" in "example/a" but a dir in "example/b"
|  |- dir "foobar" is missing in "example/b"
|  |- dir "folder"
|  |  |- file "ijk.txt" differs
```

Made with the most basic C++ standard API.

**Note: Don't use this for anything important!** - at least make a backup before you compare.

This tool is **read-only** and shouldn't modify anything... but with any quickly made project, *UB can creep in*.

I am not liable for any data loss.
