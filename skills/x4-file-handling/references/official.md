# Official Mode (FSBrowser /edit)

Use when `/status` and `/list?dir=/` exist, and the UI is `/edit`.

## Base URL

Use the device IP shown on the WiFi screen (e.g., `http://192.168.100.121`).

## List files

```sh
curl "http://DEVICE_IP/list?dir=/"
```

## Upload file

Upload uses multipart form field `data` and the filename must include the full
destination path.

```sh
curl -F "data=@book.epub;filename=/book.epub" "http://DEVICE_IP/edit"
```

Upload to a subfolder:

```sh
curl -F "data=@book.epub;filename=/Books/book.epub" "http://DEVICE_IP/edit"
```

## Create folder

Use `PUT /edit` with a path that ends in `/`.

```sh
curl -X PUT -F "path=/Books/" "http://DEVICE_IP/edit"
```

## Delete file or folder

```sh
curl -X DELETE -F "path=/Books/old.epub" "http://DEVICE_IP/edit"
```

## Rename / move

```sh
curl -X PUT -F "path=/Books/new.epub" -F "src=/Books/old.epub" "http://DEVICE_IP/edit"
```

## Status

```sh
curl "http://DEVICE_IP/status"
```
