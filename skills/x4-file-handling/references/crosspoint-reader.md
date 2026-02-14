# CrossPoint Reader Mode (new web server)

Use when `/api/status` and `/api/files` exist, and the UI is `/files`.

## Base URL

Use the device IP shown on the WiFi screen, or `http://crosspoint.local` if mDNS works.

## List files

```sh
curl "http://DEVICE_IP/api/files?path=/"
```

## Upload file

```sh
curl -F "file=@book.epub" "http://DEVICE_IP/upload?path=/"
```

Upload to a subfolder:

```sh
curl -F "file=@book.epub" "http://DEVICE_IP/upload?path=/Books"
```

## Create folder

```sh
curl -X POST -F "name=Books" -F "path=/" "http://DEVICE_IP/mkdir"
```

## Delete file or folder

```sh
curl -X POST -F "path=/Books/old.epub" "http://DEVICE_IP/delete"
```

Folders must be empty before delete.

## Rename

No rename endpoint. Re-upload with the new name, then delete the old file.

## Status

```sh
curl "http://DEVICE_IP/api/status"
```
