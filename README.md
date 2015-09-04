# Cierna nad Tisou
3 party file transfer for POSIX systems.

## Description
Each transfer is defined by a number of objects:
 * server
 * uploader
 * downloader
 * user-selected token

The uploader offers a file to a server, while providing a string token that
identifies the file. Tokens are unique within a server. The downloader later
requests a file with a token. Requesting a non-existent token does not result
in a indefinite waiting period, but gets refused immediately. After pairing the
uploader with the downloader, the file transfer begins.

## Example workflow
### Step 1: External agreement
Communication over a secure channel:
```
Alice: hey
Bob: yo!
Alice: i will send you the Harry Potter movie we spoke about
Bob: k
Alice: the token will be 'harry' 
Alice: and the password is 'dumbledore_is_gay'
Bob: send it already
```

### Step 2: Uploader (Alice)
```
$ openssl aes-256-cbc -a -salt -in hp4.mkv -out hp4.enc -k dumbledore_is_gay
$ cntup hp4.enc harry
Waiting for the downloader...
```

### Step 3: Downloader (Bob)
```
$ cntdown hp4.enc harry
$ openssl aes-256-cbc -d -a -in hp.enc -out hp4.mkv -k dumbledore_is_gay
```

## Goals
The following is a list of important design goals of the project.

### Support for big files
Transfer of files up to `2^64 bytes ~= 18 exabytes` must be supported. This
means that transfer of increasingly big files such as movies or backups is
possible with no obstructions.

### No middle copy
The server *must not* store or analyze any of the user data and therefore
respect the privacy of the users.

### Only server needs a public static IP
Nowadays, when the IPv6 is still not commonplace, sending a file directly via
`nc` might not be possible due to network architecture, e.g. NAT. A point of
contact for both sender and receiver is a public server, but neither sender nor
receiver must have a public IP.

### Highly portable client library
The client logic should be implemented in a most portable way, as a shared C
library, so that everyone can build a tool around the functionality in
arbitrary language compatible with C. 

#### Command-line client
Apart from the library, the default distribution should provide a command-line
client out-of-the-box.

### Stable high-throughput server implementation
The server code must be able to handle a large number of concurrent file
transfers.

### Publicly available free server
Not everyone has an affordable access to a machine with public static IP with
necessary permissions to run the server code. One instance of a publicly
available server free for all should run at all times.

### Open-source
Allow the users to read the code and build a trust towards the software, whilst
providing the opportunity to modify the code freely. 2-clause BSD license is a
good choice for this.

## Non-guaranteed properties
### Speed
The system does not provide any performance related guarantee.

### Security
The transfers are not secured in any way. It is *highly* advised to protect the
data beforehand with a password and to share the password with the recipient of
the data in a secure way. It is moderately possible that a fourth party would
obtain the file, in case of a successful guess of the file token.

## Licence
2-clause BSD licence. 

## Author
Daniel Lovasko daniel.lovasko@gmail.com

