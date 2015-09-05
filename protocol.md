# Protocol
## Initiation
### Offer (uploader -> server)
```C
message init_offer {
	byte code = 0
	byte token_length
	byte *token
}
```

### Offer reply (server -> uploader)
```C
message init_offer_reply {
	byte code
		0 = success
		1 = token in use
		2 = too many files, try later
}
```

### Request (downloader -> server)
```C
message init_request {
	byte code = 1
	byte file_offset[8] big endian
	byte token_length
	byte *token
}
```

### Request reply (server -> downloader)
```C
message init_request_reply {
	byte code
		0 = success
		1 = no such token
		2 = too many transfers, try later
}
```

## Transfer
### Request (server -> uploader)
```C
message trans_request {
	byte file_offset[8] big endian
	byte chunk_length[8] big endian
}
```

### Request reply (uploader -> server)
```C
message trans_request_reply {
	byte code
		0 = normal chunk
		1 = last chunk
		2 = error
	byte checksum[4] big endian
	byte data_length[8] big endian
	byte *data
}
```
In case of an `code == 2 (error)`, the `data` space is used to transfer
string description of the error, and `data_length` is the length of the string.

### Offer (server -> downloader)
```C
message trans_offer {
	byte code
		0 = normal chunk
		1 = last chunk
		2 = error
	byte data_length[8] big endian
	byte *data
}
```
Note: `trans_offer` is identical to `trans_request_reply`.

### Offer reply (downloader -> server)
```C
message trans_offer_reply {
	byte code
		0 = success
		1 = error
}
```

