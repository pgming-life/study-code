import tkinter.font
import glob as g
from natsort import natsorted
from PIL import Image, ImageTk
from pgming_package.module_gui_text import *

# 画像形式
ext = [
    'png',
    'jpg',
    'jpeg',
    'bmp',
]

# リサイズ保存フォルダパス
path_resize = "ImageGallery_resize"

# 画像ファイル読み込み
list_img = []
print("画像ファイル読み込み中...")
for i in ext:
    list_img.append(g.glob("**/*.{}".format(i), recursive=True))    # 当該ディレクトリ以下を検索

# resizeフォルダを排他
list_img = [j for i in list_img for j in i if j[:len(path_resize)] != path_resize]

# テキスト
lines_text = []
print("画像が見つかりました！！" if list_img else "画像が見つかりません！！")
lines_text.append("画像が見つかりました！！" if list_img else "画像が見つかりません！！")
lines_text.append("この実行ファイル以下のディレクトリ階層から画像を読み込んで表示します。")
lines_text.append("拡張子はpng, jpg, jpeg, bmpが対象です。")

# Guiアプリケーション
class GuiApplication(tk.Frame):
    # コンストラクタ
    def __init__(self, master=None, size={}, texts=[], images=[]):
        super().__init__(master)
        self.master = master
        self.master.title("ImageGallery")
        self.size = size
        self.texts = texts
        self.images = images
        self.pack()
        self.create_widgets()

    # ウィジェット作成
    def create_widgets(self):
        # テキスト作成
        self.font = tk.font.Font(family='Arial', size=11)
        self.text = tk.Text(self, height=10, width=80)
        self.text.configure(font=self.font)
        for i in self.texts:
            self.text.insert('end', "{}\n".format(i))
        self.text.grid(row=0, columnspan=2, sticky=(tk.N, tk.W, tk.S, tk.E))

        # テキストスクロールバー作成
        self.text_scrollbar = tk.ttk.Scrollbar(self, orient=tk.VERTICAL, command=self.text.yview)
        self.text['yscrollcommand'] = self.text_scrollbar.set
        self.text_scrollbar.grid(row=0, column=2, sticky=(tk.N, tk.S))

        # 画像リストがある場合
        if self.images:
            # リストボックス作成
            self.currencies = tuple(self.images)
            self.var_lb = tk.StringVar(value=self.currencies)
            self.listbox = tk.Listbox(self, listvariable=self.var_lb, height=35, width=50)
            self.listbox.grid(row=1, column=1)
            self.listbox.bind("<Key>", image_change_sck)

            # リストボックススクロールバー作成
            self.lb_scrollbar = tk.ttk.Scrollbar(self, orient=tk.VERTICAL, command=self.listbox.yview)
            self.listbox['yscrollcommand'] = self.lb_scrollbar.set
            self.lb_scrollbar.grid(row=1, column=2, sticky=(tk.N, tk.S))

            # ボタン作成
            self.button_ok = tk.ttk.Button(self, text='表示', padding=10)
            self.button_ok.grid(row=2, column=1, pady=20)
            self.button_ok.bind('<Button-1>', image_change)

            # キャンバス作成
            self.canvas = tk.Canvas(self, width=self.size['w'], height=self.size['h'])
            self.canvas.grid(row=1, column=0, rowspan=3)

            # キャンバスに初期画像をセット
            self.canvas.photo = ImageTk.PhotoImage(image_init)
            self.on_canvas = self.canvas.create_image(0, 0, anchor='nw', image=self.canvas.photo)

# 画像切り替えショートカットキー
def image_change_sck(event):
    # Enterキーが押された場合
    if str(event.keysym) == "Return":
        # 選択プログラム実行
        for i in app.listbox.curselection():
            img = Image.open(app.listbox.get(i))
            set_image(event, img)
    
    # Downキーが押された場合
    if str(event.keysym) == "Down":
        # 選択プログラム実行
        for i in app.listbox.curselection():
            for j, k in enumerate(app.images):
                if k == app.listbox.get(i):
                    if j != len(app.images) - 1:
                        img = Image.open(app.images[j + 1])
                        set_image(event, img)
                    break
    
    # Upキーが押された場合
    if str(event.keysym) == "Up":
        # 選択プログラム実行
        for i in app.listbox.curselection():
            for j, k in enumerate(app.images):
                if k == app.listbox.get(i):
                    if j != 0:
                        img = Image.open(app.images[j - 1])
                        set_image(event, img)
                    break

# 画像切り替え
def image_change(event):
    # 選択プログラム実行
    for i in app.listbox.curselection():
        img = Image.open(app.listbox.get(i))
        set_image(event, img)

# 画像セット
def set_image(event, img):
    # キャンバスの書き換え
    app.canvas.photo = ImageTk.PhotoImage(img)
    app.canvas.itemconfig(app.on_canvas, image=app.canvas.photo)

# フォルダ順に自然型ソート
list_img = natsorted(list_img)

# 画像MAXサイズを求める(max1200×800)
max_w, max_h = 1200, 800
width = height = 0
for i, j in enumerate(list_img):
    img = Image.open(j)
    w, h = img.size
    if w > max_w or h > max_h:
        for k in range(1, w):   # 無限ループは恐いので最大値wを取る
            if w // k <= max_w and h // k <= max_h:     # maxに収まるように整数に調整
                w = w // k
                h = h // k
                folder_create(path_resize)
                path_string = path_resize
                for m in os.path.dirname(j).split("\\"):
                    path_string += "\\" + m
                    folder_create(path_string)
                break
        img_resize = img.resize((w, h))     # リサイズ
        path = path_resize + "\\" + j       # パスルート
        img_resize.save(path)               # リサイズ画像保存
        list_img[i] = path                  # 置換
    if w > width:
        width = w
    if h > height:
        height = h

# 初期画像設定
if list_img:
    image_init = Image.open(list_img[0])

# アプリケーション起動
print("GUI起動...")
window = tk.Tk()
app = GuiApplication(master=window, size={'w' : width, 'h' : height}, texts=lines_text, images=list_img)
app.mainloop()
