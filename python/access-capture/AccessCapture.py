import os
from selenium import webdriver
from selenium.webdriver.chrome.options import Options

urls = [
	"https://pgming-ctrl.com",
    "https://pgming-ctrl.com/pgming-life",
    "https://pgming-ctrl.com/test.html",
]

# ヘッドレスモードに設定
op = Options()
op.add_argument('--headless')

# ChromeのWebドライバを使用
#driver = webdriver.Chrome("chromedriver_win32/chromedriver")              # ブラウザを起動させて取得(ページの上部から下部までのキャプチャが取得不可)
driver = webdriver.Chrome("chromedriver_win32/chromedriver", options=op)    # ヘッドレスモードが有効(ページの上部から下部までのキャプチャが取得可能)

# Chromeブラウザ経由でキャプチャを取得
for url in urls:
    # URLアクセス
    driver.get(url)

    # ページ全体を取得
    page_width = driver.execute_script('return document.body.scrollWidth')
    page_height = driver.execute_script('return document.body.scrollHeight')
    driver.set_window_size(page_width, page_height)

    # フォルダ確認・作成
    if not os.path.exists("images"):
        os.makedirs(path_folder, exist_ok=True)

    # 画像をURL名で保存
    url_name = url.replace("https://", "").replace("http://", "").replace("/", "_").replace("?", "_")
    driver.save_screenshot("images/" + str(url_name   ) + ".png")
