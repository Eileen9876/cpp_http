# cpp_http
簡易 http 基本功能實現。

## 下載 json.hpp
此處使用 [nlohmann/json](https://github.com/nlohmann/json) 以實現 JSON 資料傳輸。

## 使用方法

### Get
```cpp
Client client("127.0.0.1", 8000);
auto res = client.Get("/");
if (res->status_code == 200) cout << "Get : " << res->message_body << endl;
```
可使用 `res.Error()` 確認是否執行失敗，範例如下
```cpp
if (res.Error()) cout << res.GetErrorMsg() << endl;
```

### Post
```cpp
json msg_body;
msg_body["id"] = "ab22546";

Client client("127.0.0.1", 8000);
auto res = client.Post("/auto_mosaic", msg_body);

if (res.Error()) {
  cout << res.GetErrorMsg() << endl;
  return 1;
} 
if (res->status_code == 200) cout << "Post : " << res->message_body << endl;
```
