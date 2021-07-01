import requests
from bs4 import BeautifulSoup

# URL
url = "https://pgming-ctrl.com/test.html"

# リクエストを送信してレスポンスを取得
html = requests.get(url)
# Response [404] - ページなし
# Response [200] - ページあり

# HTML情報を取得
soup = BeautifulSoup(html.content, "html.parser")

# HTML全体を表示
output = []
#output.append(soup)

# タイトルを取得
output.append("タイトル : " + soup.find("title").text)

# 見出しを取得
h = []
h.append(soup.find_all("h1"))
h.append(soup.find_all("h2"))
h.append(soup.find_all("h3"))
h.append(soup.find_all("h4"))
h.append(soup.find_all("h5"))
h.append(soup.find_all("h6"))

string_h = [
    "見出し1  : ",
    "見出し2  : ",
    "見出し3  : ",
    "見出し4  : ",
    "見出し5  : ",
    "見出し6  : ",    
]

for i, j in enumerate(h):
    for k in j:
        output.append(string_h[i] + k.text)

# 出力
for i in output:
    print(i)

input("\n>>>\n>>>\n>>>\n>>>処理が終了しました。ウィンドウを閉じるにはEnterを押してください...\n\n")
