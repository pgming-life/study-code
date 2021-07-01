"""fwmodule_general.py"""

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
    from pgming_package.fwdef import *
else:               # デバッグ用インポート
    # ルートパス可変制御
    import sys, os      # fwdef.pyと同じモジュールインポートだが、Python内部でインクルードガード出来ているので問題ない。
    if not [i for i in sys.path if i == os.path.dirname(__file__)]:
        sys.path.append(os.path.dirname(__file__))
    from fwdef import *

#####################################################################
"""
    カウント
    備考:
    ・呼び出すごとにカウントする
    ・クラスなので定義するときに分けることが可能
    ・resultでそれまでの結果を返す

"""
class counter:
    def __init__(self, radix=0, operand=1, operator="+"):
        self.radix = radix
        self.operand = operand
        self.operator = operator
    def count(self):
        if self.operator == "+":
            self.radix += self.operand
        elif self.operator == "-":
            self.radix -= self.operand
        elif self.operator == "*":
            self.radix *= self.operand
        elif self.operator == "/":
            self.radix /= self.operand
        elif self.operator == "%":
            self.radix %= self.operand
        elif self.operator == "expo":
            self.radix **= self.operand
        elif self.operator == "root":
            self.radix **= (1 / self.operand)
        else:
            self.radix = None
        return self.radix
    def result(self):
        return self.radix

#####################################################################
"""
    テスト

"""
# ↓↓↓直接このファイルを実行したときの処理
if __name__ == "__main__":
    #from fwmodule_general import *

    a = counter()
    x = 2
    b = counter(x, 2, "*")
    for i in range(10):
        c = counter()
        a.count()
        b.count()
        c.count()
    print(a.result())
    print(b.result())       # 1024にはならず、べき乗を狙っても1回多い値になる
    print(int(b.result() / x))  # なので一回分減らす
    print(c.result())

    input(">>>\n>>>\n>>>\n処理が終了しました。\n")
    