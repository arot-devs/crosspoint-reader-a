---
name: x4-file-handling
description: Manage files on the X4 e-reader over WiFi. Use when listing, uploading, deleting, or renaming files via the device web APIs in either CrossPoint Reader mode (/api/*, /upload, /delete, /mkdir) or Official/FSBrowser mode (/edit, /list, /status).
---

# X4 File Handling

## Decide mode

1) **CrossPoint Reader mode**  
   Use when `GET /api/status` works or the web UI is `/files`.  
   See `references/crosspoint-reader.md`.

2) **Official (FSBrowser) mode**  
   Use when `GET /status` and `GET /list?dir=/` work and the web UI is `/edit`.  
   See `references/official.md`.

## Use references

- **CrossPoint Reader operations**: list, upload, delete, create folder, rename workaround  
  `references/crosspoint-reader.md`
- **Official (FSBrowser) operations**: list, upload, delete, rename, create folder  
  `references/official.md`
