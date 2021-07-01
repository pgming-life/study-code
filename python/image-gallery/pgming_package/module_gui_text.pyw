"""module_gui_text.pyw"""

# テキスト処理メモ
"""
文字列処理は正規表現の方が処理速度は速いが、想定外の文字列が入り強制終了したりするバグの温床にもなり得るため、
それに、変更を加えるときに見づらく書きづらい(細かいことをいちいち調べる必要がある)ためmodule_textにあるstring_pickクラスを使い統一する。
この統一をすることで一貫性と柔軟性の両方を備えることができており非常にコードが書きやすくなる。
想定外のバグを回避するためにもstring_pickで文字数の方を操作することで想定内の安定した処理を施せる。

"""

import collections as cl

flag_release = False

try:
    import pgming_package.release as r
except Exception:
    # エラースルー
    pass
else:
    flag_release = r.flag

# 上記のインポートが読み込めない場合はflag_releaseは基本的にFalse(存在しない変数であるため)
if flag_release:    # リリース用インポート
    from pgming_package.fwmodule_gui_progress import *
    from pgming_package.charcode import *
else:               # デバッグ用インポート
    # ルートパス可変制御
    import sys, os      # fwdef.pyと同じモジュールインポートだが、Python内部でインクルードガード出来ているので問題ない。
    if not [i for i in sys.path if i == os.path.dirname(__file__)]:
        sys.path.append(os.path.dirname(__file__))
    from fwmodule_gui_progress import *
    from charcode import *

#####################################################################
"""
    フォルダの確認・作成
    ex) folder_create("フォルダパス", ・・・)
    
"""
def folder_create(path_folder):
    text_gui = ""

    # フォルダの有無を確認
    flag_none = False
    if not os.path.exists(path_folder):
        flag_none = True
    else:
        text_gui = "フォルダを確認しました。{}".format(path_folder)

    # 無いフォルダに対して新規作成
    if flag_none:
        os.makedirs(path_folder, exist_ok=True)
        text_gui = "フォルダを作成しました。{}".format(path_folder)

    # ネームドタプルに格納
    result = cl.namedtuple('result', 'flag, text')

    return result(flag=False if flag_none else True, text=text_gui)

#####################################################################
"""
    ファイルの確認・作成
    備考:
    ・記述する内容がなければ省略可能。(何も記述しない)
    ex) file_create("ファイルパス", ファイルの中に記述する内容[行ずつ格納したリスト], '作成エンコード名')
    
"""
def file_create(path_file, lines_string=[], ecd=list_charcode[0]):
    text_gui = ""
    
    # ファイルの有無を確認
    flag_none = False
    if not os.path.exists(path_file):
        flag_none = True
    else:
        text_gui = "ファイルを確認しました。{}".format(path_file)

    # 無いファイルに対して新規作成
    if flag_none:
        with open(path_file, 'w', encoding=ecd) as f:
            for line in lines_string:
                f.writelines("{}\n".format(line))
        text_gui = "ファイルを作成しました。{}".format(path_file)

    # ネームドタプルに格納
    result = cl.namedtuple('result', 'flag, text')

    return result(flag=False if flag_none else True, text=text_gui)

#####################################################################
"""
    ファイル・フォルダ確認(警告終了)
    ex) path_search_end("ファイルパス・フォルダパス")
    
"""
def path_search_end(path):
    text_gui = ""

    # ファイル・フォルダの有無を確認
    flag_none = False
    if not os.path.exists(path):
        flag_none = True
    else:
        text_gui = "確認しました。{}".format(path)

    # 無いファイル・フォルダに対して警告し終了
    if flag_none:
        text_gui = "ありません。処理を終了します。{}".format(path)
        # Falseだった時にエラー視認のためにtime.sleep(5)推奨

    # ネームドタプルに格納
    result = cl.namedtuple('result', 'flag, text')
    
    return result(flag=False if flag_none else True, text=text_gui)

#####################################################################
"""
    ファイル・フォルダ確認(警告続行)
    ex) path_search_continue("ファイルパス・フォルダパス")
    
"""
def path_search_continue(path):
    text_gui = ""

    # ファイル・フォルダの有無を確認
    flag_none = False
    if not os.path.exists(path):
        flag_none = True
    else:
        text_gui = "確認しました。{}".format(path)

    # 無いファイル・フォルダに対して警告し続行
    if flag_none:
        text_gui = "ありませんが、処理を続行します。{}".format(path)
        # Falseだった時にエラー視認のためにtime.sleep(5)推奨

    # ネームドタプルに格納
    result = cl.namedtuple('result', 'flag, text')
    
    return result(flag=False if flag_none else True, text=text_gui)

#####################################################################
"""
    ファイル内容全てを行リストとして読み込む
    ex) file_readlines("ファイルパス", '読み込みエンコード名')
    
"""
def file_readlines(path_file, ecd=''):
    flag_none = False
    text_gui = ""
    list_line = []

    # 文字コードが分かる場合
    if ecd:
        try:
            with open(path_file, encoding=ecd) as f:
                list_line = f.readlines()
                list_line = [line.rstrip() for line in list_line]
        except Exception:
            # エラースルー
            pass

    # 文字コード総当たりでファイルの読み込み
    if not ecd:
        for i, j in enumerate(list_charcode):
            try:
                with open(path_file, encoding=j) as f:
                    list_line = f.readlines()
                    list_line = [line.rstrip() for line in list_line]
            except Exception as err:
                #print("\n{}, file open error".format(j))
                #print("{}\ncontinue...".format(err))
                if i == len(list_charcode) - 1:
                    flag_none = True
                    text_gui = "読み込めません。{}".format(path_file)
                    # Falseだった時にエラー視認のためにtime.sleep(5)推奨
            else:
                break

    # ネームドタプルに格納
    result = cl.namedtuple('result', 'flag, text, line')

    return result(flag=False if flag_none else True, text=text_gui, line=list_line)

#####################################################################
"""
    入力された文字列から指定したファイル内を検索しその文字列がある行番号かつ行リストを取得
    ex) lines_list("文字列", "ファイルパス", '読み込みエンコード名')
    
"""
def lines_list(string, path_file, ecd=''):
    flag_none = False
    text_gui = ""
    text = ""

    # 文字コードが分かる場合
    if ecd:
        try:
            with open(path_file, encoding=ecd) as f:
                text = f.read()
        except Exception as err:
            # エラースルー
            pass

    # 文字コード総当たりでファイルの読み込み
    if not ecd:
        for i, j in enumerate(list_charcode):
            try:
                with open(path_file, encoding=j) as f:
                    text = f.read()
            except Exception as err:
                #print("\n{}, file open error".format(j))
                #print("{}\ncontinue...".format(err))
                if i == len(list_charcode) - 1:
                    flag_none = True
                    text_gui = "読み込めません。{}".format(path_file)
                    # Falseだった時にエラー視認のためにtime.sleep(5)推奨
            else:
                break

    list_num = []
    list_line = []
    if not flag_none:
        # ファイルの読み込み
        lines = file_readlines(path_file, ecd if ecd else '')[2]

        # 行番号取得
        num = 0
        for line in text.splitlines():
            num += 1
            if string in line:
                list_num.append(num)

        # ラベルを含む行を取得
        for i in list_num:
            list_line.append(lines[i - 1])

    # ネームドタプルに格納
    result = cl.namedtuple('result', 'flag, text, num, line')
    
    return result(flag=False if flag_none else True, text=text_gui, num=list_num, line=list_line)

#####################################################################
"""
    文字列操作
    備考: 文字列を入れて操作する
    ex) string_pick(lines_list(string, path)[i])

"""
class string_pick:
    #=====================================
    # 初期値
    def __init__(self, line):
        self.line = line

    #=====================================
    """
        指定された行(単なる文字列)から指定した文字列を開始位置sから探索しその文字列番号(シフトはt)からu番目(取り出したい文字列数)までの文字列を取得
        備考:
        ・文字列探索はその行にその文字列が一つだけしかない場合とする(これは.pickの場合で.setは探索開始位置mからn回目の文字列からを取得可能)
        ・s, tを省略するためにNoneと入力した場合は探索された文字列の0番目から取り出すとする
        ・u以上を省略した場合は文字列の後尾までとなる
        ・flag_u_strnum=Trueの場合、uは取り出したい文字列数となる(flag_u_strnum=Falseの場合は文字列の先頭からu番目となる)
        ex) .pick("文字列", s, t, u=.set("文字列", m, n))

    """
    def pick(self, string, s=None, t=None, u=None, flag_u_strnum=False):
        if s is not None:
            s = self.line.find(string, s)           # 探索開始位置+sシフト
        else:
            s = self.line.find(string)              # 探索開始位置0
        if t is not None:
            if u is not None:
                if flag_u_strnum:                   # flag_u_strnum=True : +sのt,uの間(文字列数分取り出す)
                    t += s
                    u += t
                    name_string = self.line[t:u]
                else:                               # flag_u_strnum=True : +sのtから文字列の先頭からのuの間
                    t += s
                    name_string = self.line[t:u]
            else:                                   # flag_u_strnum=False : +sのtから文字列の後尾まで
                t += s
                name_string = self.line[t:]
        elif u is not None:
            if flag_u_strnum:                       # flag_u_strnum=True : 現在sから+sのuの間(文字列数分取り出す)
                u += s
                name_string = self.line[s:u]
            else:                                   # flag_u_strnum=False : 文字列の先頭からのsから文字列の先頭からのuの間
                name_string = self.line[s:u]
        else:                                       # flag_u_setに関わらずsから文字列の後尾まで
            name_string = self.line[s:]
        return name_string

    #=====================================
    """
        n回目の指定した文字列を開始位置mから探索しその文字列番号を取得する
        備考:
        ・主に上記のs,t,uに対して使用する(s,t,u以外で使うとなると直接的な文字列の取り出しに使用する ex)string[.set("文字列", m, n):.set("文字列", m, n)])
        ・uに対して使用する場合、flag_u_strnum=Falseにする(.setは文字列数ではなく頭からの文字列番号を返すため)
        ・0回目は1回目として数えられる
        ・n回目の文字列がない場合は-1が返る
        ・nを省略した場合は１回目を取る
        ex1) .set("文字列", m, n)
    
    """
    def set(self, string, m=None, n=1):         # 文字列の先頭からの+mしたn回目の文字列番号
        if m is not None:
            m = self.line.find(string, m)       # 探索開始位置+mシフト
        else:
            m = self.line.find(string)          # 探索開始位置0
        for i in range(1, n):
            m = self.line.find(string, m + 1)   # n回目文字列番号へ上書き
        return m

#####################################################################
"""
    テスト

"""
# ↓↓↓直接このファイルを実行したときの処理
if __name__ == "__main__":
    #from module_gui_text import *

    """
    # ウィジェット配置デバッグ用
    def debug_conf(ev):
        print(ev)
    """
    
    # テスト媒体
    path_folder = "{}\\__pycache__".format(os.path.dirname(__file__))
    path_file = "{}\\test_text.cpp".format(os.path.dirname(__file__))   # DirectX12の初期化プログラムをテスト媒体にする
    string0 = "//"          # コメント行を対象
    string1 = "return"      # returnの中身を対象
    string2 = "FAILED"      # FAILEDの中身の関数引数を対象

    # オープンファイル
    try:
        # cmdから同期型でエクスプローラー経由で開く
        subprocess.run(r"explorer {}".format(path_file))
        # 本システムでのエラーを避けるためWindowsに委ねるという意図もあるが、何より起動速度が速いため採用している。
    except Exception:
        # エラースルー
        pass

    # 処理ターゲット
    class ProcessingTarget:
        # コンストラクタ
        def __init__(self, progress_x, progress_y, label_x, label_y):
            self.receiver = ProgressReceiver(progress_x, progress_y)
            self.label_progress = ProgressLabel(label_x, label_y)

        # ターゲット
        def target(self):
            # folder_create
            self.label_progress.update(folder_create(path_folder).text)     # フォルダの確認・作成
            time.sleep(0.2)         # 文字列が見えるようにウェイトを挿入

            # file_create
            self.label_progress.update(file_create(path_file).text)     # ファイルの確認・作成
            time.sleep(0.2)

            # path_search_end
            self.label_progress.update(path_search_end(path_file).text)     # ファイル・フォルダ確認(警告終了)
            time.sleep(0.2)

            # path_search_continue
            self.label_progress.update(path_search_continue(path_file).text)    # ファイル・フォルダ確認(警告続行)
            time.sleep(0.2)

            # file_readlines
            flag, text, lines = file_readlines(path_file)   # ファイル内容全てを行リストとして読み込む
            self.label_progress.update(flag)
            time.sleep(0.2)
            self.label_progress.update(text)
            time.sleep(0.2)
            for i in lines:
                self.label_progress.update(i)
                time.sleep(0.02)

            # lines_list
            flag, text, list_num, list_line = lines_list(string0, path_file)    # 入力された文字列から指定したファイル内を検索しその文字列がある行番号かつ行リストを取得
            self.label_progress.update(flag)
            time.sleep(0.2)
            self.label_progress.update(text)
            time.sleep(0.2)
            for i in range(len(list_num)):
                self.label_progress.update(list_num[i])
                time.sleep(0.02)
                self.label_progress.update(list_line[i])
                time.sleep(0.02)

            # string_pick
            list = lines_list(string1, path_file)   # 指定された文字列から後に指定した文字列を開始位置から探索しその文字列番号から取り出したい文字列数までの文字列を取得
            for i in list[3]:   # list[3] == list.line
                s = string_pick(i)
                name_string = i[s.set(string1):s.set(";")] if s.set(";") != -1 else i[s.set(string1):]
                self.label_progress.update(name_string)
                time.sleep(0.2)
                name_string = s.pick(string1, None, len(string1)+1, s.set(";"))
                self.label_progress.update(name_string)
                time.sleep(0.2)
                name_string = s.pick(string1, None, len(string1)+1, 1, flag_u_strnum=True)
                self.label_progress.update(name_string)
                time.sleep(0.2)

            # string_pick - 関数引数を再帰的に読み込む
            list = lines_list(string2, path_file)
            for i in list.line:
                s = string_pick(i)
                func_or_val = s.pick("(", s.set(string2), 1, s.set("(", s.set(string2), 2) if s.set("(", s.set(string2), 2) != -1 else s.set(")"))
                self.label_progress.update("Name: " + func_or_val)
                time.sleep(0.2)
                if s.set("(", s.set(string2), 2) != -1:
                    m = string_pick(s.pick("(", s.set(s.pick("(", s.set(string2), 1, s.set("(", s.set(string2), 2)))))
                    args = m.pick("(", None, 1, m.set(")", None, m.pick("").count("(")))
                    self.label_progress.update("Args: " + args)
                    time.sleep(0.2)
                    if "," in args:
                        # ループ処理
                        n = string_pick(args)
                        for j in range(1, len(args)):   # 無限ループは恐いので対象文字列の長さ分をMAXとしてループ
                            arg = args[(0 if j == 1 else n.set(",", None, j-1) + 1):(n.set(",", None, j) if n.set(",", None, j) != -1 else len(args))]
                            self.label_progress.update("Arg: " + arg.strip())
                            time.sleep(0.2)
                            if n.set(",", None, j) == -1:
                                break
                        # 下記の方が一括の場合は簡単で速いですが、上記であれば複雑怪奇でパターンが乏しい文字列でもその都度同じアルゴリズムによって処理できるような応用さがあります。
                        """
                        for j in args.split(","):
                            self.label_progress.update("Arg:  " + j.strip())
                            time.sleep(0.2)
                        """

            # スレッド終了処理
            self.receiver.flag_loop = False
            self.label_progress.end("", flag_dt=True, flag_timer=True)
        
        # スレッドスタート
        def start(self):
            self.thread_target = threading.Thread(target = self.target)
            self.thread_target.setDaemon(True)
            self.thread_target.start()

    # GUIアプリケーション
    class GuiApplication(tk.Frame):
        # コンストラクタ
        def __init__(self, master=None):
            window_width = 900
            window_height = 500

            # Frameクラスを継承
            super().__init__(master, width=window_width, height=window_height)
            
            # 初期値代入
            self.master = master
            self.master.title("GUIテキスト処理テスト")
            self.master.minsize(window_width, window_height)
            self.pack()
            #self.bind('<Configure>', debug_conf)

            # ターゲット処理
            self.target = ProcessingTarget(progress_x=350, progress_y=50, label_x=100, label_y=20)

            # ウィジェット作成
            self.create_widgets()

        # ウィジェット作成
        def create_widgets(self):
            # ボタン作成
            self.button_start = tk.ttk.Button(self, text="開始", padding=10, command=self.start_event)
            self.button_start.place(x=100, y=100)

        # 処理開始
        def start_event(self):
            if not self.target.receiver.flag_loop:     # スレッド重複処理の防止
                self.target.receiver.flag_loop = True
                self.target.receiver.flag_progress = False
                self.target.receiver.start()
                self.target.start()
    
    # アプリケーション起動
    window = tk.Tk()
    app = GuiApplication(master=window)
    app.mainloop()
