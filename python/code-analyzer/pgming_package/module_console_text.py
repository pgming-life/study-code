"""module_console_text.py"""

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
    from pgming_package.fwmodule_console_progress import *
    from pgming_package.charcode import *
else:               # デバッグ用インポート
    # ルートパス可変制御
    import sys, os      # fwdef.pyと同じモジュールインポートだが、Python内部でインクルードガード出来ているので問題ない。
    if not [i for i in sys.path if i == os.path.dirname(__file__)]:
        sys.path.append(os.path.dirname(__file__))
    from fwmodule_console_progress import *
    from charcode import *

#####################################################################
"""
    フォルダの確認・作成
    ex) folder_create("フォルダパス", ・・・)
    
"""
def folder_create(path_folder):
    # フォルダの有無を確認
    flag_none = False
    if not os.path.exists(path_folder):
        flag_none = True
    else:
        print("フォルダを確認しました。{}".format(path_folder))

    # 無いフォルダに対して作成するかどうかを質問して作成
    if flag_none:
        input("\nフォルダがありません。{0}\nフォルダを作成しますか？{0}\n作成する場合はEnterを押してください...".format(path_folder))
        os.makedirs(path_folder, exist_ok=True)
        print("「{}」フォルダを作成しました。".format(path_folder))
    
    return False if flag_none else True

#####################################################################
"""
    ファイルの確認・作成
    備考:
    ・記述する内容がなければ省略可能。(何も記述しない)
    ex) file_create("ファイルパス", ファイルの中に記述する内容[行ずつ格納したリスト], '作成エンコード名')
    
"""
def file_create(path_file, lines_string=[], ecd=list_charcode[0]):
    # ファイルの有無を確認
    flag_none = False
    if not os.path.exists(path_file):
        flag_none = True
    else:
        print("ファイルを確認しました。{}".format(path_file))

    # 無いファイルに対して作成するかどうかを質問して作成
    if flag_none:
        input("\nファイルがありません。{0}\nファイルを作成しますか？{0}\n作成する場合はEnterを押してください...".format(path_file))
        with open(path_file, 'w', encoding=ecd) as f:
            for line in lines_string:
                f.writelines("{}\n".format(line))
        print("ファイルを作成しました。{}".format(path_file))

    return False if flag_none else True

#####################################################################
"""
    ファイル・フォルダ確認(警告終了)
    備考:
    ・どんな引数でも可能[タプル(args), 辞書(kwargs)]
    ex) path_search_end("ファイルパス", "ファイルパス", "フォルダパス", ・・・)
    
"""
def path_search_end(*args, **kwargs):
    # ファイル・フォルダの有無を確認
    list_none = []
    for i in args:
        if not os.path.exists(i):
            list_none.append(i)
        else:
            print("確認しました。{}".format(i))

    for i in kwargs:
        if not os.path.exists(i):
            list_none.append(i)
        else:
            print("確認しました。{}".format(i))

    # 無いファイル・フォルダに対して警告し終了
    if list_none:
        print()
        for i in list_none:
            print("ありません。{}".format(i))
        input("確認してください。\n処理を終了します。このウィンドウを閉じてください。\nウィンドウを閉じるにはEnterを押してください...")
        sys.exit()

#####################################################################
"""
    ファイル・フォルダ確認(警告続行)
    ex) path_search_continue("ファイルパス・フォルダパス")
    
"""
def path_search_continue(path):
    # ファイル・フォルダの有無を確認
    flag_none = False
    if not os.path.exists(path):
        flag_none = True
    else:
        print("確認しました。{}".format(path))

    # 無いファイル・フォルダに対して警告し続行
    if flag_none:
        print("\nありません。{}".format(path))
        input("処理を続行しますか？\n続行するにはEnterを押してください...")

    return False if flag_none else True

#####################################################################
"""
    ファイル内容全てを行リストとして読み込む
    ex) file_readlines("ファイルパス", '読み込みエンコード名')
    
"""
def file_readlines(path_file, ecd=''):
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
                    print("読み込めません。{}".format(path_file))
            else:
                break

    return list_line

#####################################################################
"""
    入力された文字列から指定したファイル内を検索しその文字列がある行番号かつ行リストを取得
    ex) lines_list("文字列", "ファイルパス", '読み込みエンコード名')
    
"""
def lines_list(string, path_file, ecd=''):
    flag_none = False

    # 文字コードが分かる場合
    if ecd:
        try:
            with open(path_file, encoding=ecd) as f:
                text = f.read()
        except Exception:
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
                    print("読み込めません。{}".format(path_file))
                    flag_none = True
            else:
                break

    list_num = []
    list_line = []
    if not flag_none:
        # ファイルの読み込み
        lines = file_readlines(path_file, ecd if ecd else '')

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
    result = cl.namedtuple('result', 'num, line')

    return result(num=list_num, line=list_line)

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
    #from module_console_text import *

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

    @class_decolater
    def main_func():
        # folder_create
        folder_create(path_folder)  # フォルダの確認・作成

        # file_create
        file_create(path_file)      # ファイルの確認・作成

        # path_search_end
        path_search_end(path_file)  # ファイル・フォルダ確認(警告終了)

        # path_search_continue
        flag = path_search_continue(path_file)  # ファイル・フォルダ確認(警告続行)
        print("結果: {}".format(flag))
        python_connection()

        # file_readlines
        lines = file_readlines(path_file)   # ファイル内容全てを行リストとして読み込む
        for i in lines:
            print(i)
        python_connection()
        
        # lines_list
        list = lines_list(string0, path_file)   # 入力された文字列から指定したファイル内を検索しその文字列がある行番号かつ行リストを取得
        for i, j in enumerate(list[0]): # list[0] == list.num
            print(list.num[i])
            print(list.line[i])
        python_connection()

        # string_pick
        list = lines_list(string1, path_file)
        for i in list.line:
            s = string_pick(i)  # 指定された文字列から後に指定した文字列を開始位置から探索しその文字列番号から取り出したい文字列数までの文字列を取得
            print("----------------------")
            name_string = i[s.set(string1):(s.set(";") if s.set(";") != -1 else len(i))]
            print(name_string)
            name_string = s.pick(string1, None, len(string1)+1, s.set(";"))
            print(name_string)
            name_string = s.pick(string1, None, len(string1)+1, 1, flag_u_strnum=True)
            print(name_string)
        print("----------------------")
        python_connection()
        
        # string_pick - 関数引数を再帰的に読み込む
        list = lines_list(string2, path_file)
        for i in list.line:
            s = string_pick(i)
            func_or_val = s.pick("(", s.set(string2), 1, s.set("(", s.set(string2), 2) if s.set("(", s.set(string2), 2) != -1 else s.set(")"))
            print("----------------------")
            print("Name: " + func_or_val)
            if s.set("(", s.set(string2), 2) != -1:
                m = string_pick(s.pick("(", s.set(s.pick("(", s.set(string2), 1, s.set("(", s.set(string2), 2)))))
                args = m.pick("(", None, 1, m.set(")", None, m.pick("").count("(")))
                print("Args: " + args)
                if "," in args:
                    # ループ処理
                    n = string_pick(args)
                    for j in range(1, len(args)):   # 無限ループは恐いので対象文字列の長さ分をMAXとしてループ
                        arg = args[(0 if j == 1 else n.set(",", None, j-1) + 1):(n.set(",", None, j) if n.set(",", None, j) != -1 else len(args))]
                        print("Arg:  " + arg.strip())
                        if n.set(",", None, j) == -1:
                            break
                    # 下記の方が一括の場合は簡単で速いですが、上記であれば複雑怪奇でパターンが乏しい文字列でもその都度同じアルゴリズムによって処理できるような応用さがあります。
                    """
                    for j in args.split(","):
                        print("Arg:  " + j.strip())
                    """
        print("----------------------")

    # メイン関数実行
    main_func()
