# 🧠 Multithreaded HTTP Server in C

A lightweight multithreaded HTTP server written in C using POSIX sockets.  
Built step-by-step to understand low-level networking, HTTP protocol handling, and systems design.

---

## 🚀 Features

### ✅ Core Networking
- TCP server using `socket`, `bind`, `listen`, `accept`
- Handles multiple client connections

### ✅ HTTP Support
- Parses HTTP/1.1 requests
- Supports:
  - `GET`
  - `POST`
- Extracts:
  - Method
  - Path
  - Headers
  - Body

### ✅ Static File Serving
- Serves files from `/static` directory
- MIME type detection (HTML, CSS, JS, images, etc.)
- Efficient file streaming using `fread + send`

### ✅ POST Handling
- Supports `application/x-www-form-urlencoded`
- Parses form data into key-value pairs
- URL decoding (`%XX`, `+` → space)

### ✅ Routing System
- Custom route table
- Maps:
```

method + path → handler function

```
- Supports dynamic request handling

### ✅ Header System
- Parses and stores all HTTP headers
- Case-insensitive lookup via `get_header()`

### ✅ Response Abstraction
- Centralized response builder:
```

send_response()
send_ok()
send_404()
send_500()

```
- Proper HTTP formatting

### ✅ Security
- Prevents **path traversal attacks**
```

/../../etc/passwd ❌ blocked

```
- Uses `realpath()` to validate filesystem access

### ✅ Hardening
- Handles malformed requests safely
- Limits:
- header size
- body size
- Validates:
- request line
- Content-Length
- Prevents crashes and undefined behavior

### ✅ Concurrency
- Multi-threaded server using `pthread`
- Handles multiple clients in parallel
- Uses:
- `pthread_create`
- `pthread_detach`

---

## 📂 Project Structure

```

http-server/
│
├── src/
│   ├── main.c
│   ├── server.c
│   ├── handler.c
│   ├── parser.c
│   ├── response.c
│   ├── file.c
│
├── include/
│   ├── server.h
│   ├── handler.h
│   ├── parser.h
│   ├── response.h
│   ├── file.h
│
├── static/
│   └── index.html
│
├── Makefile
├── README.md
└── .gitignore

````

---

## ⚙️ Build & Run

### Build

```bash
make
````

### Run

```bash
./http_server
```

Server runs on:

```
http://localhost:8080
```

---

## 🧪 Testing

### GET Request

```bash
curl http://localhost:8080/
```

---

### POST Request (Form Data)

```bash
curl -X POST http://localhost:8080/submit \
     -H "Content-Type: application/x-www-form-urlencoded" \
     -d "name=Mamun&age=25"
```

---

### Concurrency Test

```bash
for i in {1..10}; do curl http://localhost:8080/ & done
```

---

### Security Test

```bash
curl http://localhost:8080/../../etc/passwd
```

Expected:

```
Forbidden
```

---

## 🧠 Key Learnings

This project demonstrates:

* Low-level socket programming
* HTTP protocol internals
* Stream-based parsing
* Memory management in C
* Defensive programming (hardening)
* Threading and concurrency
* Secure file handling

---

## 🙌 Acknowledgment

Built as a systems learning project to deeply understand how web servers work under the hood.

---

## 📜 License

MIT License