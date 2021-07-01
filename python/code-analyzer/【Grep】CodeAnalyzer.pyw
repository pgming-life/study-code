import json
import glob as g
import pandas as pd
import tkinter.font
from natsort import natsorted

# PgmingPackageインポート
from pgming_package import module_gui_text as gui

# タグ辞書
tag_list = {'sch' : "search", 'path' : "path", 'ext' : "extension"}

# データ保存先
file_config = "grep_config.json"

# ファイル確認・作成
def file_write_create():
    lines = []
    lines.append("{")
    lines.append("    \"inputbox\": {")
    lines.append("        \"{}\": [],".format(tag_list['sch']))
    lines.append("        \"{}\": [],".format(tag_list['path']))
    lines.append("        \"{}\": []".format(tag_list['ext']))
    lines.append("    }")
    lines.append("}")
    gui.file_create(file_config, lines_string=lines)
file_write_create()

flag_err = False
try:
    # JSONファイルからテキストストリームを生成
    with open(file_config, 'r') as f:
        # 辞書オブジェクト(dictionary)を取得(JSON形式)
        data_json = json.load(f)
except Exception:
    # JSON形式で読み込めなかった場合のエラー回避
    flag_err = True

# 要素解析
rate_search = [] if flag_err else data_json['inputbox'][tag_list['sch']]
rate_path = [] if flag_err else data_json['inputbox'][tag_list['path']]
rate_ext = [] if flag_err else data_json['inputbox'][tag_list['ext']]

"""
# ウィジェット配置デバッグ用
def debug_conf(ev):
    print(ev)
"""

# 処理ターゲット
class ProcessingTarget:
    # コンストラクタ
    def __init__(self, self_root, progress_x, progress_y, label_x, label_y, label_move_x, label_move_y, label_move_len, bar_x, bar_y, bar_len):
        self.df = []                    # データテーブル
        self.searchs = []               # 検索対象
        self.num_string_sch = []        # 検索対象文字列の行番号
        self.num_line_sch = []          # 検索対象行の行番号
        self.file_sch = []              # 検索対象ファイル整合性リスト
        self.num_file_line_sch = []     # 検索対象ファイルの検索文字列行番号
        self.receiver = gui.ProgressReceiver(progress_x, progress_y)
        self.label_progress = gui.ProgressLabel(label_x, label_y)
        self.label_progress_move = gui.MoveProgressLabel(label_move_x, label_move_y, label_move_len)
        self.progressbar = gui.Progressbar(self_root, bar_x, bar_y, bar_len)

    # ターゲット
    def target(self, raw_searchs, raw_paths, raw_exts):
        # 初期化
        flag_error = False
        output_error = []
        self.df = [[] for i in range(5)]    # データフレーム領域確保(pandas格納数)
        self.num_string_sch = []
        self.num_line_sch = []
        self.file_sch = []
        self.num_file_line_sch = []
        update_widgets()

        # ";"で区切られる複数の検索対象の抽出
        self.searchs = raw_searchs
        paths = []
        exts = []
        for i in raw_paths:
            if ";" in i:
                s = i.split(";")
                for j in s:
                    if j != "":
                        paths.append(j)
            else:
                paths.append(i)
        for i in raw_exts:
            if ";" in i:
                s = i.split(";")
                for j in s:
                    if j != "":
                        exts.append(j)
            else:
                exts.append(i)

         # 動的３次元配列のヒープ領域確保(パス×拡張子)
        list_file = [[[] for i in exts] for i in paths]

        # ３次元配列への穴埋め
        self.label_progress.update("対象ファイル列挙中...")
        for num_path, rate_path in enumerate(paths):
            for num_ext, rate_ext in enumerate(exts):
                if rate_path[0:3] != "\\/*":    # このパターン以外を受け付ける(終了できないデッドロック状態に陥るため)
                    if rate_ext[0:2] == "*.":   # このパターンだけ受け付ける(「*」だけだとフォルダ名も検索してしまうので「*」と「.」でセットで受け付ける)
                        # 当該パス・相対パス・絶対パス対応
                        list_file[num_path][num_ext] = g.glob("{0}{1}".format("**/" if rate_path == "\\" else rate_path[1:] + "/**/" if rate_path[0] == "\\" else rate_path + "/**/", rate_ext), recursive=True)

        # int型正規表現ソート
        for num_path, rate_path in enumerate(list_file):
            for num_ext, rate_ext in enumerate(rate_path):
                list_file[num_path][num_ext] = natsorted(rate_ext)
                # 最初から辞書順にソートされているリストだが、
                # 自然順にソートされているわけではないのでソートし直す。

        # フラグ
        # flag_path, flag_ext, flag_sch, flag_once

        # Grep検索し上から対象パス、検索対象、対象拡張子、ファイル順に特定行を抽出
        text_search = "検索中..."
        output = []
        cnt_output = gui.counter()
        cnt_file_sch = gui.counter()
        output.append("[paths]")
        cnt_output.count()
        for num_path, rate_path in enumerate(paths):
            flag_path = False
            if num_path != len(paths) - 1:
                flag_path = True
            output.append("    |")
            cnt_output.count()
            output.append("    |-{}".format(rate_path))
            cnt_output.count()
            output.append("    |[extensions]")
            cnt_output.count()
            for num_ext, rate_ext in enumerate(exts):
                self.label_progress.update("{0}ファイル{1}「{2}/**/{3}」".format(len(list_file[num_path][num_ext]), text_search, rate_path, rate_ext))
                flag_ext = False
                if num_ext != len(exts) - 1:
                    flag_ext = True
                output.append("    {0}    |".format("|" if flag_path else " "))
                cnt_output.count()
                output.append("    {0}    |-{1}".format("|" if flag_path else " ", rate_ext))
                cnt_output.count()
                output.append("    {0}    |[files]".format("|" if flag_path else " "))
                cnt_output.count()
                if list_file[num_path][num_ext]:
                    # ファイル解析
                    self.progressbar.set.configure(maximum=len(list_file[num_path][num_ext]))
                    for num_file, rate_file in enumerate(list_file[num_path][num_ext]):
                        self.progressbar.update(num_file)
                        flag_sch = False
                        flag_once = False
                        for num_sch, rate_sch in enumerate(self.searchs):
                            flag, text, list_num, list_line = gui.lines_list(rate_sch, rate_file)
                            if not flag:
                                flag_error = True
                                output_error.append(text)
                                self.label_progress.update(text)
                                self.label_progress.update("{0}ファイル{1}「{2}/**/{3}」".format(len(list_file[num_path][num_ext]), text_search, rate_path, rate_ext))
                            if list_line:
                                flag_sch = True
                                if not flag_once:
                                    output.append("    {0}    {1}    |".format("|" if flag_path else " ", "|" if flag_ext else " "))
                                    cnt_output.count()
                                    output.append("    {0}    {1}    |-{2}".format("|" if flag_path else " ", "|" if flag_ext else " ", rate_file))
                                    cnt_output.count()
                                    output.append("    {0}    {1}    |[lines]".format("|" if flag_path else " ", "|" if flag_ext else " "))
                                    cnt_output.count()
                                    flag_once = True
                                output.append("    {0}    {1}    |    |↓[searchs]{2}".format("|" if flag_path else " ", "|" if flag_ext else " ", rate_sch))
                                cnt_output.count()
                                self.num_string_sch.append(cnt_output.result())
                                for num_line, rate_line in enumerate(list_line):
                                    output.append("    {0}    {1}    |    |>{4}{2}<|{3}".format("|" if flag_path else " ", "|" if flag_ext else " ", list_num[num_line], rate_line, "0" * (7 - len(str(list_num[num_line])))))
                                    cnt_output.count()
                                    self.num_line_sch.append(cnt_output.result())
                                    self.file_sch.append([])
                                    self.file_sch[cnt_file_sch.result()].append(cnt_output.result())
                                    self.file_sch[cnt_file_sch.result()].append(rate_file)
                                    self.num_file_line_sch.append([])
                                    self.num_file_line_sch[cnt_file_sch.result()].append(cnt_output.result())
                                    self.num_file_line_sch[cnt_file_sch.result()].append(list_num[num_line])
                                    cnt_file_sch.count()
                                # リストへ格納
                                cnt_array = gui.counter()
                                self.df[cnt_array.result()].append(rate_path)
                                self.df[cnt_array.count()].append(rate_ext)
                                self.df[cnt_array.count()].append(rate_file)
                                self.df[cnt_array.count()].append(rate_sch)
                                self.df[cnt_array.count()].append(list_line)
                        if flag_sch:
                            output.append("    {0}    {1}    |-----------------".format("|" if flag_path else " ", "|" if flag_ext else " "))
                            cnt_output.count()

        # リストから入れ替えでデータテーブルへ格納
        cnt_array = gui.counter()
        self.df = pd.DataFrame({'path' : self.df[cnt_array.result()], 'ext' : self.df[cnt_array.count()], 'file' : self.df[cnt_array.count()], 'sch' : self.df[cnt_array.count()], 'line' : self.df[cnt_array.count()]})

        # エラーログ出力
        if flag_error:
            self.label_progress.update("読み込みエラーがありました。")
            gui.time.sleep(.5)
            self.label_progress.update("エラーログ[grep_error.log]を出力しています。")
            gui.time.sleep(.5)
            with open("grep_error.log", "w") as f:
                self.progressbar.set.configure(maximum=len(output_error))
                for num_line, rate_line in enumerate(output_error):
                    self.progressbar.update(num_line)
                    f.writelines("{}\n".format(rate_line))
            self.label_progress.update("エラーログ[grep_error.log]を開きます。")
            try:
                # cmdから非同期でエクスプローラー経由で開く
                gui.subprocess.Popen(args=['explorer', r'grep_error.log'], shell=True)
                # 本システムでのエラーを避けるためWindowsに委ねるという意図もあるが、何より起動速度が速いため採用している。
            except gui.subprocess.CalledProcessError:
                self.label_progress.update("エラーログ[grep_error.log]を開けません。")
                gui.time.sleep(.5)
                self.label_progress.update("終了します...")
                gui.time.sleep(.5)

        # ウィジェット更新
        update_widgets(output, self.searchs, self.num_string_sch, self.num_line_sch)

        # スレッド終了処理      
        self.receiver.flag_loop = False
        self.label_progress_move.flag_loop = False

        # ラベルリセット
        self.label_progress.end("", flag_dt=True, flag_timer=True)
        self.label_progress_move.end()

    # スレッドスタート
    def start(self, searchs, paths, exts):
        self.thread_target = gui.threading.Thread(target = self.target, args=(searchs, paths, exts,))   # 次開始で上書き
        self.thread_target.setDaemon(True)                                                          # デーモンスレッドに設定
        self.thread_target.start()                                                                  # 処理開始
        # デーモン化することでメインスレッド(Tkinter)が強制終了するとサブスレッド(thread_target)は自動的に破棄される。
        # (thread_targetに紐づいているrecieverも同様に破棄)

# ウィジェット更新
def update_widgets(output=[], searchs=[], num_string_sch=[], num_line_sch=[]):
    # ツリーリストボックス更新
    app.tab2.listbox_tree.delete(0, gui.tk.END)     # 初期化
    app.tab2.var_lb_tree = gui.tk.StringVar(value=tuple(output))
    app.tab2.listbox_tree['listvariable'] = app.tab2.var_lb_tree
    if output:
        for i in num_string_sch:
            app.tab2.listbox_tree.itemconfig(i - 1, foreground='blue violet')
        for i in num_line_sch:
            app.tab2.listbox_tree.itemconfig(i - 1, foreground='forest green')

    # ファイルリストボックス更新
    app.tab2.listbox_file.delete(0, gui.tk.END)     # 初期化
    if output:
        if not app.tab1.target.df.empty:
            app.tab2.var_lb_file = gui.tk.StringVar(value=natsorted(tuple(set([i for i in app.tab1.target.df.file]))))  # 重複しないファイルリストを代入
            app.tab2.listbox_file['listvariable'] = app.tab2.var_lb_file

    # ファイルテキストボックス更新
    app.tab2.text_file.delete('1.0', 'end')     # 初期化
    if output:
        if not app.tab1.target.df.empty:
            app.tab2.text_file.tag_config("line_sch", background='green4')
            app.tab2.text_file.tag_add('line_sch', 'insert linestart', 'insert lineend')
            app.tab2.text_file.tag_config("string_sch", background='dark violet')
            app.tab2.text_file.tag_add('string_sch', 'insert linestart', 'insert lineend')
            file0_lines = gui.file_readlines(list(natsorted(set([i for i in app.tab1.target.df.file])))[0]).line
            for i, j in enumerate(file0_lines):
                if [k for k in searchs if k in j]:
                    app.tab2.text_file.insert('end', "|>{1}{0}<|".format(i + 1, "0" * (len(str(len(output))) - len(str(i + 1)))), 'line_sch')
                    p = 0
                    for k in searchs:
                        if k in j:
                            app.tab2.text_file.insert('end', j[p:j.find(k)], 'line_sch')
                            app.tab2.text_file.insert('end', j[j.find(k):j.find(k)+len(k)], 'string_sch')
                            p = j.find(k) + len(k)
                    app.tab2.text_file.insert('end', "{}\n".format(j[p:]), 'line_sch')
                else:
                    app.tab2.text_file.insert('end', "|>{2}{0}<|{1}\n".format(i + 1, j, "0" * (len(str(len(output))) - len(str(i + 1)))))

# 検索タブ
class SearchTab(gui.tk.Frame):
    # コンストラクタ
    def __init__(self, master=None, searchs=[], paths=[], exts=[]):
        # Frameクラスを継承
        super().__init__(master)

        # 動的オブジェクトメモリ確保
        self.input_search = []
        self.input_path = []
        self.input_ext = []

        # 初期値代入
        self.searchs = searchs
        self.paths = paths
        self.exts = exts

        # 処理ターゲット
        self.target = ProcessingTarget(
            self,
            progress_x=280,
            progress_y=330,
            label_x=30,
            label_y=35,
            label_move_x=18,
            label_move_y=730,
            label_move_len=30,
            bar_x=30,
            bar_y=40,
            bar_len=517,
        )

        # ウィジェット作成
        self.create_widgets()

    # ウィジェット作成
    def create_widgets(self):
        # 検索文字列ウィジェットの位置、幅、数
        search_label_x = 30
        search_label_y = 120
        search_box_width = 30
        search_box_x = 30
        search_box_y = 150
        search_box_y_margin = 29
        search_box_num = 20

        # パスウィジェットの位置、幅、数
        path_label_x = 245
        path_label_y = 120
        path_box_width = 50
        path_box_x = 245
        path_box_y = 150
        path_box_y_margin = 30
        path_box_num = 1

        # 拡張子ウィジェットの位置、幅、数
        ext_label_x = 245
        ext_label_y = 185
        ext_box_width = 50
        ext_box_x = 245
        ext_box_y = 215
        ext_box_y_margin = 30
        ext_box_num = 1

        # ラベル作成
        self.label_search = gui.tk.Label(text="Grep検索文字列：")
        self.label_search.place(x=search_label_x, y=search_label_y)
        self.label_note = gui.tk.Label(text="パスも拡張子も末尾の「;」の有無はどちらでも可")
        self.label_note.place(x=280, y=245)
        self.label_path = gui.tk.Label(text="パス： ex)C:\Program Files;\;\my_package;   ※当該・相対パス可")
        self.label_path.place(x=path_label_x, y=path_label_y)
        self.label_ext = gui.tk.Label(text="拡張子：ex)*.xml;*.json;*.py;*.pyw;*.txt   ※「*.」は必須")
        self.label_ext.place(x=ext_label_x, y=ext_label_y)

        # インプットボックス作成
        self.input_search.append(gui.tk.Entry(width=search_box_width))
        self.input_search[0].place(x=search_box_x, y=search_box_y)
        if self.searchs:
            self.input_search[0].insert(gui.tk.END, self.searchs[0])
        p = gui.counter(search_box_y, search_box_y_margin)
        for i in range(1, search_box_num):
            p.count()
            self.input_search.append(gui.tk.Entry(width=search_box_width))
            self.input_search[i].place(x=search_box_x, y=p.result())
            if i < len(self.searchs):
                self.input_search[i].insert(gui.tk.END, self.searchs[i])
        for i in self.input_search:
            i.bind("<Key>", search_start_sck)
        self.input_path.append(gui.tk.Entry(width=path_box_width))
        self.input_path[0].place(x=path_box_x, y=path_box_y)
        if self.paths:
            self.input_path[0].insert(gui.tk.END, self.paths[0])
        p = gui.counter(path_box_y, path_box_y_margin)
        for i in range(1, path_box_num):
            p.count()
            self.input_path.append(gui.tk.Entry(width=path_box_width))
            self.input_path[i].place(x=path_box_x, y=p.result())
            if i < len(self.paths):
                self.input_path[i].insert(gui.tk.END, self.paths[i])
        for i in self.input_path:
            i.bind("<Key>", search_start_sck)
        self.input_ext.append(gui.tk.Entry(width=ext_box_width))
        self.input_ext[0].place(x=ext_box_x, y=ext_box_y)
        if self.exts:
            self.input_ext[0].insert(gui.tk.END, self.exts[0])
        p = gui.counter(ext_box_y, ext_box_y_margin)
        for i in range(1, ext_box_num):
            p.count()
            self.input_ext.append(gui.tk.Entry(width=ext_box_width))
            self.input_ext[i].place(x=ext_box_x, y=p.result())
            if i < len(self.exts):
                self.input_ext[i].insert(gui.tk.END, self.exts[i])
        for i in self.input_ext:
            i.bind("<Key>", search_start_sck)

        # ボタン作成
        self.button_ok = gui.tk.ttk.Button(self, text='検索開始', padding=10, command=self.get_input)
        self.button_ok.place(x=345, y=250)

    # 入力文字列取得
    def get_input(self):
        searchs = []
        paths = []
        exts = []
        for i in self.input_search:
            if i.get() != "":
                searchs.append(i.get())
        for i in self.input_path:
            if i.get() != "":
                paths.append(i.get())
        for i in self.input_ext:
            if i.get() != "":
                exts.append(i.get())

        # 入力データ保存
        SaveInputData(searchs, paths, exts)

        if not self.target.receiver.flag_loop:      # スレッド重複処理の防止
            if searchs and paths and exts:
                # 検索開始
                GrepSearch(searchs, paths, exts)
            else:
                self.target.label_progress.update("３つの入力項目を全て埋めてください。")

# 検索開始ショートカットキー
def search_start_sck(event):
    # Enterキーが押された場合
    if event.keysym == 'Return':
        app.tab1.get_input()

# Grep検索
def GrepSearch(searchs, paths, exts):
    app.tab1.target.receiver.flag_loop = True
    app.tab1.target.receiver.flag_progress = False
    app.tab1.target.receiver.start()
    app.tab1.target.label_progress_move.flag_loop = True
    app.tab1.target.label_progress_move.start(" Searching. ")
    app.tab1.target.progressbar.reset()
    app.tab1.target.start(searchs, paths, exts)

# 入力データ保存
def SaveInputData(searchs, paths, exts):
    # ファイル確認・作成
    file_write_create()

    # データ加工
    data = gui.cl.OrderedDict()
    data[tag_list['sch']] = [i for i in searchs]
    data[tag_list['path']] = [i for i in paths]
    data[tag_list['ext']] = [i for i in exts]
    ys = gui.cl.OrderedDict()
    ys["inputbox"] = data
    # ファイルに書き込む際は、万が一、他のプログラムからファイルを読み込むことになると
    # 順番が入れ替わってしまうことによる不具合が起きる可能性があるので、
    # collections.OrderedDictで順序を指定した辞書を作成し書き込む。

    # JSONファイルへ出力
    with open(file_config, 'w') as fw:
        json.dump(ys, fw, indent=4)

# 結果タブ
class ResultTab(gui.tk.Frame):
    # コンストラクタ
    def __init__(self, master=None, data_tree=[], data_file=[], texts_file=[]):
        # Frameクラスを継承
        super().__init__(master)

        # 初期値代入
        self.data_tree = []
        self.data_file = []
        if data_tree:
            for i in data_tree:
                for j in i.file:
                    self.data_tree.append(j)
        if data_file:
            for i in data_file:
                for j in i.file:
                    self.data_file.append(j)

        # ウィジェット作成
        #######################################################################################
        # ツリーリストボックス作成
        self.font_tree = gui.tk.font.Font(family=u'メイリオ', size=10)
        self.listbox_tree = gui.tk.Listbox(self, height=10, background='gray3', foreground='lime green')
        self.listbox_tree.configure(font=self.font_tree)
        #self.listbox_tree.insert(gui.tk.END, *self.data_tree)
        for i in self.data_tree:
            self.listbox_tree.insert(gui.tk.END, "{}\n".format(i))
        self.listbox_tree.bind("<Key>", listbox_tree_event_sck)
        self.listbox_tree.pack(expand=True, fill=gui.tk.BOTH, side=gui.tk.TOP)

        # ファイルリストボックス作成
        self.var_lb_file = gui.tk.StringVar(value=tuple(set(self.data_file)))   # 重複しないデータとして渡す
        self.listbox_file = gui.tk.Listbox(self, listvariable=self.var_lb_file, height=1, background='gray3', foreground='lime green')
        self.listbox_file.bind("<Key>", listbox_file_event_sck)
        self.listbox_file.pack(expand=True, fill=gui.tk.BOTH, side=gui.tk.TOP)

        # ファイルテキストボックス作成
        self.font_file = gui.tk.font.Font(family=u'メイリオ', size=10)
        self.text_file = gui.tk.Text(self, wrap=gui.tk.NONE, height=23, width=50, background='gray17', foreground='lavender', insertbackground='lavender')
        self.text_file.configure(font=self.font_file)
        for i in texts_file:
            self.text_file.insert('end', "{}\n".format(i))
        self.text_file.pack(expand=True, fill=gui.tk.BOTH, side=gui.tk.LEFT)

        # ファイルテキストボックススクロールバー作成
        self.text_file_scrollbar = gui.tk.ttk.Scrollbar(self, orient=gui.tk.VERTICAL, command=self.text_file.yview)
        self.text_file['yscrollcommand'] = self.text_file_scrollbar.set
        self.text_file_scrollbar.pack(expand=True, fill=gui.tk.Y)

# ツリーリストボックスイベントショートカットキー
def listbox_tree_event_sck(event):
    # Enterキーが押された場合
    if event.keysym == 'Return':
        # 選択プログラム実行
        for a in app.tab2.listbox_tree.curselection():
            for b, c in enumerate(app.tab1.target.file_sch):
                if a + 1 == c[0]:
                    _, _, output = gui.file_readlines(c[1])

                    # ウィジェット更新
                    #######################################################################################
                    # ファイルテキストボックス更新
                    app.tab2.text_file.delete('1.0', 'end')
                    for i, j in enumerate(output):
                        if [k for k in app.tab1.target.searchs if k in j]:
                            if i + 1 == app.tab1.target.num_file_line_sch[b][1]:
                                app.tab2.text_file.insert('end', "|>{1}{0}<|".format(i + 1, "0" * (len(str(len(output))) - len(str(i + 1)))), 'line_sch')
                                p = 0
                                for k in app.tab1.target.searchs:
                                    if k in j:
                                        app.tab2.text_file.insert('end', j[p:j.find(k)], 'line_sch')
                                        app.tab2.text_file.insert('end', j[j.find(k):j.find(k)+len(k)], 'string_sch')
                                        p = j.find(k) + len(k)
                                app.tab2.text_file.insert('end', "{}\n".format(j[p:]), 'line_sch')
                            else:
                                app.tab2.text_file.insert('end', "|>{1}{0}<|".format(i + 1, "0" * (len(str(len(output))) - len(str(i + 1)))))
                                p = 0
                                for k in app.tab1.target.searchs:
                                    if k in j:
                                        app.tab2.text_file.insert('end', j[p:j.find(k)])
                                        app.tab2.text_file.insert('end', j[j.find(k):j.find(k)+len(k)], 'string_sch')
                                        p = j.find(k) + len(k)
                                app.tab2.text_file.insert('end', "{}\n".format(j[p:]))
                        else:
                            app.tab2.text_file.insert('end', "|>{2}{0}<|{1}\n".format(i + 1, j, "0" * (len(str(len(output))) - len(str(i + 1)))))

                    # 指定行へジャンプ
                    app.tab2.text_file.see('{}'.format(float(app.tab1.target.num_file_line_sch[b][1])))

# ファイルリストボックスイベントショートカットキー
def listbox_file_event_sck(event):
    # Enterキーが押された場合
    if event.keysym == 'Return':
        # 選択プログラム実行
        for i in app.tab2.listbox_file.curselection():
            _, _, output = gui.file_readlines(app.tab2.listbox_file.get(i))

            # ウィジェット更新
            #######################################################################################
            # ファイルテキストボックス更新
            app.tab2.text_file.delete('1.0', 'end')
            for i, j in enumerate(output):
                if [k for k in app.tab1.target.searchs if k in j]:
                    app.tab2.text_file.insert('end', "|>{1}{0}<|".format(i + 1, "0" * (len(str(len(output))) - len(str(i + 1)))), 'line_sch')
                    p = 0
                    for k in app.tab1.target.searchs:
                        if k in j:
                            app.tab2.text_file.insert('end', j[p:j.find(k)], 'line_sch')
                            app.tab2.text_file.insert('end', j[j.find(k):j.find(k)+len(k)], 'string_sch')
                            p = j.find(k) + len(k)
                    app.tab2.text_file.insert('end', "{}\n".format(j[p:]), 'line_sch')
                else:
                    app.tab2.text_file.insert('end', "|>{2}{0}<|{1}\n".format(i + 1, j, "0" * (len(str(len(output))) - len(str(i + 1)))))
    
    # oキーが押された場合
    if event.keysym == 'o':
        # 選択プログラム実行
        for i in app.tab2.listbox_file.curselection():
            try:
                # cmdから同期型でエクスプローラー経由で開く
                gui.subprocess.run("explorer {}".format(app.tab2.listbox_file.get(i)))
                # 本システムでのエラーを避けるためWindowsに委ねるという意図もあるが、何より起動速度が速いため採用している。
            except Exception:
                # エラースルー
                pass

# GUIアプリケーション
class GuiApplication(gui.tk.ttk.Notebook):
    # コンストラクタ
    def __init__(self, master=None, searchs=[], paths=[], exts=[]):
        window_width = 575
        window_height = 750

        # Frameクラスを継承
        super().__init__(master, width=window_width, height=window_height)

        # 初期値代入
        self.master = master
        self.master.title("【Grep】CodeAnalyzer")
        #self.master.minsize(window_width, window_height)
        self.pack(expand=True, fill=gui.tk.BOTH)
        #self.bind('<Configure>', debug_conf)

        # 動的オブジェクトメモリ確保
        self.input_search = []
        self.input_path = []
        self.input_ext = []

        # タブ作成
        self.create_tabs()

    # タブ作成
    def create_tabs(self):
        # 検索タブ
        self.tab1 = SearchTab(master=self.master, searchs=rate_search, paths=rate_path, exts=rate_ext)
        self.add(self.tab1, text="/               検索               /")
        
        # 結果タブ
        self.tab2 = ResultTab(master=self.master)
        self.add(self.tab2, text="/               結果               /")

# アプリケーション起動
window = gui.tk.Tk()
app = GuiApplication(master=window, searchs=rate_search, paths=rate_path, exts=rate_ext)
app.mainloop()
