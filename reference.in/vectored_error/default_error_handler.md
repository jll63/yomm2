<sub>/ ->home / ->reference / ->vectored_error / ->vectored_error/default_error_handler </sub>

# vectored_error<Policy>::**default_error_handler**

---
```
static void default_error_handler(const error_type& err);
```

If ->`error_output` is available in `Policy`, use it to print a description of
`err`. Return normally, causing the program to be aborted by the caller.

**Parameters**

**err** - the variant containing the error.

**Return value**

(none)
