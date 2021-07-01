#IfWinActive

;グローバル変数
global g_indent_space := 
global g_switch := 
global g_case_num := 
global g_break := 0
global g_name := 
global g_public := 1
global g_private := 1
global g_protected := 0
global g_switch_flag := False
global g_class_flag := False
global g_active_id := 
global g_tab :=
global g_tab_space := 

;ヘルプ
^+F1::
    WinGet, active_id, ID, A
    MsgBox, ＜ヘルプ＞`n`n------------------------------------------------------------------------------------------`n`n・if`nCtrl + Shift + I`n`n・else if`nCtrl + Shift + Alt + E`n`n・else`nCtrl + Shift + E`n`n・switch`nCtrl + Shift + Alt + S`n`n・for`nCtrl + Shift + F`n`n・while`nCtrl + Shift + W`n`n・struct`nCtrl + Shift + T`n`n・enum`nCtrl + Shift + Y`n`n・class`nCtrl + Shift + U`n`n------------------------------------------------------------------------------------------`n`n・インデントスペースの登録`nCtrl + Shift + N`n`n・インデントの出力`nCtrl + Shift + Enter`n`n※`n「インデントスペースの登録」は、「Shift + Home」のキーを送信し先頭スペースをコピーする。`nそれにより、「インデントの出力」で改行してからコピーした先頭スペースをペーストする。`n`n------------------------------------------------------------------------------------------`n`n・先頭に１つのタブを追加`nCtrl + Shift + J`n`n・１つのタブのスペース数を登録`nCtrl + Shift + Alt + K`n`n・先頭の１つのタブスペースを削除`nCtrl + Shift + K`n`n※`nタブスペース操作のJとKのホットキーはコピペによるインデントスペースの調整に用い、`n続けて各行で調整できるように処理の最後に「Down」と「Home」のキーを送信している。`n長押しや早い連打は厳禁。`n`n------------------------------------------------------------------------------------------`n`n・処理途中停止`nF8長押し
    WinActivate, ahk_id %active_id%
    Return

;if文
^+I::
    ;現在のアクティブウィンドウのIDを取得
    WinGet, active_id, ID, A
    
    ;条件定義共通入力関数
    def := InputDefine(active_id)
    If(!def)
        Return
    
    ;F8長押しで処理停止
    If(!CancelProcess(False))
        Return
    
    ;最初に取得したIDに該当するウィンドウをアクティブ
    WinActivate, ahk_id %active_id%
    ;インプットダイアログボックスを閉じたときに、
    ;意図せず他のウィンドウがアクティブになることを防ぐため。
    
    If(!CancelProcess(False))
        Return
    
    ;半角英数字に切り替え
    IMEGetState()
    
    If(!CancelProcess(False))
        Return
    
    ;クリップボードのバックアップ
    backup_cb = %ClipboardAll%
    
    ;クリップボードをクリア
    Clipboard = 
    ;クリップするものが空だと、
    ;バックアップされているクリップを参照してしまうため、
    ;クリップボードをクリアする。
    
    ;▼▼▼出力▼▼▼
    ;構文共通出力
    If(!SyntaxCommon(active_id, backup_cb, "if(", def))
        Return
    
    ;バックアップをクリップボードに戻す
    Clipboard = %backup_cb%
    
    Return

;else if
^+!E::
    WinGet, active_id, ID, A
    def := InputDefine(active_id)
    If(!def)
        Return
    If(!CancelProcess(False))
        Return
    WinActivate, ahk_id %active_id%
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;▼▼▼出力▼▼▼
    If(!SyntaxCommon(active_id, backup_cb, "else if(", def))
        Return
    
    Clipboard = %backup_cb%
    Return

;else
^+E::
    WinGet, active_id, ID, A
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;▼▼▼出力▼▼▼
    If(!SyntaxCommon(active_id, backup_cb, "else"))
        Return
    
    Clipboard = %backup_cb%
    Return

;switch文
^+!S::
    WinGet, active_id, ID, A
    g_active_id := active_id
    g_switch_flag := True
    g_class_flag := False
    Gui, Add, Text
    Gui, Add, Text, , switchする変数を入力してください。
    Gui, Add, Edit, W200 vg_switch, val
    Gui, Add, Text
    Gui, Add, Text, , caseの数を入力してください。
    Gui, Add, Edit, W200 vg_case_num, 3
    Gui, Add, Text
    Gui, Add, Checkbox, xm vg_break, 「break;」の有無
    If(g_break)
        GuiControl, , g_break, 1
    Gui, Add, Text
    Gui, Add, Button, W100 X25 Default , OK
    Gui, Add, Button, W100 X+0, Cancel
    Gui, Show
    Return

;for文
^+F::
    WinGet, active_id, ID, A
    def := InputDefine(active_id)
    If(!def)
        Return
    If(!CancelProcess(False))
        Return
    WinActivate, ahk_id %active_id%
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;▼▼▼出力▼▼▼
    If(!SyntaxCommon(active_id, backup_cb, "for(", def))
        Return
    
    Clipboard = %backup_cb%
    Return

;while文
^+W::
    WinGet, active_id, ID, A
    def := InputDefine(active_id)
    If(!def)
        Return
    If(!CancelProcess(False))
        Return
    WinActivate, ahk_id %active_id%
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;▼▼▼出力▼▼▼
    If(!SyntaxCommon(active_id, backup_cb, "while(", def))
        Return
    
    Clipboard = %backup_cb%
    Return

;構造体
^+T::
    WinGet, active_id, ID, A
    Loop
    {
        InputBox, name, , 構造体名を入力してください。, , , , , , , , StructName
        If(ErrorLevel <> 0)
        {
            WinActivate, ahk_id %id%
            Return
        }
        If(name == "" || RegExMatch(name, "\W"))
        {
            MsgBox, 単語構成文字([0-9a-zA-Z_])の構造体名を入力してください。
        }
        Else
        {
            Break
        }
    }
    If(!CancelProcess(False))
        Return
    WinActivate, ahk_id %active_id%
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;▼▼▼出力▼▼▼
    If(!SyntaxCommon(active_id, backup_cb, "struct", name))
        Return
    
    Clipboard = %backup_cb%
    Return

;列挙体
^+Y::
    WinGet, active_id, ID, A
    Loop
    {
        InputBox, name, , 列挙体名を入力してください。, , , , , , , , EnumName
        If(ErrorLevel <> 0)
        {
            WinActivate, ahk_id %id%
            Return
        }
        If(name == "" || RegExMatch(name, "\W"))
        {
            MsgBox, 単語構成文字([0-9a-zA-Z_])の列挙体名を入力してください。
        }
        Else
        {
            Break
        }
    }
    If(!CancelProcess(False))
        Return
    WinActivate, ahk_id %active_id%
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;▼▼▼出力▼▼▼
    If(!SyntaxCommon(active_id, backup_cb, "enum", name))
        Return
    
    Clipboard = %backup_cb%
    Return

;クラス
^+U::
    WinGet, active_id, ID, A
    g_active_id := active_id
    g_class_flag := True
    g_switch_flag := False
    Gui, Add, Text
    Gui, Add, Text, , クラス名を入力してください。
    Gui, Add, Edit, W200 vg_name, ClassName
    Gui, Add, Text
    Gui, Add, Checkbox, xm vg_public, 「public:」の有無
    Gui, Add, Checkbox, y+5 vg_private, 「private:」の有無
    Gui, Add, Checkbox, y+5 vg_protected, 「protected:」の有無
    If(g_public)
        GuiControl, , g_public, 1
    If(g_private)
        GuiControl, , g_private, 1
    If(g_protected)
        GuiControl, , g_protected, 1
    Gui, Add, Text
    Gui, Add, Button, W100 X25 Default , OK
    Gui, Add, Button, W100 X+0, Cancel
    Gui, Show
    Return
    
;インデントスペースの登録
^+!N::
    WinGet, active_id, ID, A
    backup_cb = %ClipboardAll%
    Clipboard = 
    If(!CancelProcess(True, cb))
        Return
    WinActivate, ahk_id %id%
    Send, +{Home}
    If(!CancelProcess(True, cb))
        Return
    WinActivate, ahk_id %id%
    Send, ^c
    If(!CancelProcess(True, cb))
        Return
    WinActivate, ahk_id %id%
    Send, {End}
    g_indent_space = %ClipboardAll%
    MsgBox, インデントスペースを登録しました。
    WinActivate, ahk_id %id%
    Clipboard = %backup_cb%
    Return

;インデントの出力
^+Enter::
    WinGet, active_id, ID, A
    If(!g_indent_space)
    {
        MsgBox, インデントスペースが登録されていません。`n登録して下さい。
        WinActivate, ahk_id %active_id%
        Return
    }
    backup_cb = %ClipboardAll%
    Clipboard = 
    If(!CancelProcess(True, backup_cb))
        Return
    WinActivate, ahk_id %active_id%
    Send, {Enter}
    If(!CancelProcess(True, backup_cb))
        Return
    WinActivate, ahk_id %active_id%
    Send, +{Home}
    If(!OutputString(active_id, backup_cb, g_indent_space))
        Return
    Clipboard = %backup_cb%
    Return

;タブの出力回数を登録
^+!J::
    WinGet, active_id, ID, A
    Loop
    {
        InputBox, tab, , タブの出力回数を入力してください。
        If(ErrorLevel <> 0)
        {
            WinActivate, ahk_id %active_id%
            Return
        }
        ;「入力なし」か「半角数字以外の1文字」が引っかかればやり直し
        If(tab == "" || RegExMatch(tab, "\D"))
        {
            MsgBox, 半角の自然数を入力してください。
        }
        Else
        {
            Break
        }
    }
    g_tab := tab
    MsgBox, タブの出力回数を登録しました。
    WinActivate, ahk_id %active_id%
    Return

;先頭にタブを追加(※長押しや早い連打は正常に動作せず、途中で「j」が入力される)
^+J::
    If(!g_tab)
    {
        WinGet, active_id, ID, A
        Loop
        {
            InputBox, tab, , タブの出力回数を入力してください。
            If(ErrorLevel <> 0)
            {
                WinActivate, ahk_id %active_id%
                Return
            }
            ;「入力なし」か「半角数字以外の1文字」が引っかかればやり直し
            If(tab == "" || RegExMatch(tab, "\D"))
            {
                MsgBox, 半角の自然数を入力してください。
            }
            Else
            {
                Break
            }
        }
        g_tab := tab
        MsgBox, タブの出力回数を登録しました。`n処理を続行します。
        WinActivate, ahk_id %active_id%
    }
    Loop, %g_tab%
    {
        Send, {Tab}
    }
    Send, {Down}{Home}
    Return

;１つのタブのスペース数を登録
^+!K::
    WinGet, active_id, ID, A
    Loop
    {
        InputBox, tab_space, , １つのタブのスペース数を入力してください。
        If(ErrorLevel <> 0)
        {
            WinActivate, ahk_id %active_id%
            Return
        }
        ;「入力なし」か「半角数字以外の1文字」が引っかかればやり直し
        If(tab_space == "" || RegExMatch(tab_space, "\D"))
        {
            MsgBox, 半角の自然数を入力してください。
        }
        Else
        {
            Break
        }
    }
    g_tab_space := tab_space
    MsgBox, １つのタブのスペース数を登録しました。
    WinActivate, ahk_id %active_id%
    Return

;先頭の１つのタブスペースを削除(※長押しや早い連打は正常に動作せず、途中で「k」が入力される)
^+K::
    If(!g_tab_space)
    {
        WinGet, active_id, ID, A
        Loop
        {
            InputBox, tab_space, , １つのタブのスペース数を入力してください。
            If(ErrorLevel <> 0)
            {
                WinActivate, ahk_id %active_id%
                Return
            }
            ;「入力なし」か「半角数字以外の1文字」が引っかかればやり直し
            If(tab_space == "" || RegExMatch(tab_space, "\D"))
            {
                MsgBox, 半角の自然数を入力してください。
            }
            Else
            {
                Break
            }
        }
        g_tab_space := tab_space
        MsgBox, １つのタブのスペース数を登録しました。`n処理を続行します。
        WinActivate, ahk_id %active_id%
    }
    Loop, %g_tab_space%
    {
        Send, {Delete}
    }
    Send, {Down}{Home}{Home}
    Return

;GUIボタン結果
ButtonOK:
    ;変数に反映
    Gui, Submit
    
    ;GUI解放
    Gui, Destroy
    
    ;switch文
    If(g_switch_flag)
    {
        ;「入力なし」か「単語構成文字([0-9a-zA-Z_])以外の1文字」が引っかかればやり直し
        If(g_switch == "" || RegExMatch(g_switch, "\W"))
        {
            MsgBox, 単語構成文字([0-9a-zA-Z_])のswitch変数を入力してください。
            
            ;「入力なし」か「半角数字以外の1文字」が引っかかればやり直し
            If(g_case_num == "" || RegExMatch(g_case_num, "\D"))
                MsgBox, 半角の自然数を入力してください。
            
            MsgBox, ホットキー押下からやり直してください。
            Return
        }
        
        If(g_case_num == "" || RegExMatch(g_case_num, "\D"))
        {
            MsgBox, 半角の自然数を入力してください。
            MsgBox, ホットキー押下からやり直してください。
            Return
        }
        
        ;要素が空の配列を作成
        case_array := Object()
        
        i := 1
        Loop
        {
            ;分岐する変数を昇順でインプット
            InputBox, case%i%, , %i%つ目の分岐変数を入力してください。
            case_array.Insert(case%i%)
            If(ErrorLevel <> 0)
            {
                WinActivate, ahk_id %g_active_id%
                Return
            }
            
            ;「入力なし」か「単語構成文字([0-9a-zA-Z_])以外の1文字」が引っかかればやり直し
            If(case%i% == "" || RegExMatch(case%i%, "\W"))
            {
                MsgBox, 半角英数字を入力してください。`n%i%つ目に戻ります。
                Continue
            }
            
            If(i == g_case_num)
            {
                Break
            }
            
            i++
        }
        
        If(!CancelProcess(False))
            Return
        WinActivate, ahk_id %g_active_id%
        If(!CancelProcess(False))
            Return
        IMEGetState()
        If(!CancelProcess(False))
            Return
        backup_cb = %ClipboardAll%
        Clipboard = 
        
        ;▼▼▼出力▼▼▼
        If(!SyntaxCommon(g_active_id, backup_cb, "switch(", g_switch))
            Return
        Clipboard = 
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, +{Home}
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, ^c
        front_clip = %ClipboardAll%
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, {End}
        c := "case"
        s := " "
        x := ":"
        b := "break`;"
        d := "default:"
        
        ;イテレータでcaseを出力(インデックスは1で始まる)
        For index, element in case_array
        {
            clip = %c%%s%%element%%x%
            If(!OutputString(g_active_id, backup_cb, clip))
                Return
            If(!CancelProcess(True, backup_cb))
                Return
            WinActivate, ahk_id %g_active_id%
            Send, {Enter}
            If(!CancelProcess(True, backup_cb))
                Return
            WinActivate, ahk_id %g_active_id%
            Send, +{Home}
            If(!OutputString(g_active_id, backup_cb, front_clip))
                Return
            If(g_break)
            {
                If(!CancelProcess(True, backup_cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Tab}
                If(!OutputString(g_active_id, backup_cb, b))
                    Return
                If(!CancelProcess(True, backup_cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Enter}
                If(!CancelProcess(True, backup_cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, +{Home}
                If(!OutputString(g_active_id, backup_cb, front_clip))
                    Return
            }
        }
        
        If(!OutputString(g_active_id, backup_cb, d))
            Return
        Loop, %g_case_num%
        {
            If(g_break)
            {
                If(!CancelProcess(True, backup_cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Up 2}
            }
            Else
            {
                If(!CancelProcess(True, backup_cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Up}
            }
        }
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, {End}
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, {Enter}
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, +{Home}
        If(!OutputString(g_active_id, backup_cb, front_clip))
            Return
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, {Tab}
        ;▲▲▲出力▲▲▲
    }
    ;クラス
    Else If(g_class_flag)
    {
        ;「入力なし」か「単語構成文字([0-9a-zA-Z_])以外の1文字」が引っかかればやり直し
        If(g_name == "" || RegExMatch(g_name, "\W"))
        {
            MsgBox, 単語構成文字([0-9a-zA-Z_])のクラス名を入力してください。
            MsgBox, ホットキー押下からやり直してください。
            Return
        }
        
        If(!CancelProcess(False))
            Return
        WinActivate, ahk_id %g_active_id%
        If(!CancelProcess(False))
            Return
        IMEGetState()
        If(!CancelProcess(False))
            Return
        backup_cb = %ClipboardAll%
        Clipboard = 
        
        ;▼▼▼出力▼▼▼
        If(!SyntaxCommon(g_active_id, backup_cb, "class", g_name))
            Return
    }
    
    Clipboard = %backup_cb%
    Return
ButtonCancel:
GuiClose:
    Gui, Destroy
    Return

;if(else if)文、for文、while文の条件定義共通入力関数
InputDefine(id)
{
    Loop
    {
        ;インプット
        InputBox, def, , 条件定義を入力してください。
        
        ;キャンセル処理
        If(ErrorLevel <> 0)
        {
            ;ダイアログボックスを閉じたら処理終了
            WinActivate, ahk_id %id%
            Return False
        }
        
        ;「入力なし」が引っかかればやり直し
        If(def == "")
        {
            MsgBox, 何か入力してください。
        }
        Else
        {
            Break
        }
    }
    
    Return def
}

;if(else if, else)文、switch文、for文、while文[, struct, enum, class]の構文共通出力関数
SyntaxCommon(id, cb, str, inputstr:="")
{
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +{Home}
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, ^c
    front_clip = %ClipboardAll%
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {End}
    s := " "
    b := (str == "else") ? "" : ")"
    If(str == "class" || str == "struct" || str == "enum")
    {
        clip = %str%%s%%inputstr%
    }
    Else
    {
        clip = %str%%inputstr%%b%
    }
    If(!OutputString(id, cb, clip))
        Return False
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {Enter}
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +{Home}
    If(!OutputString(id, cb, front_clip))
        Return False
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +[
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {Enter}
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +{Home}
    If(!OutputString(id, cb, front_clip))
        Return False
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {Enter}
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +{Home}
    If(!OutputString(id, cb, front_clip))
        Return False
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +]
    If(str == "struct" || str == "enum" || str == "class")
    {
        If(!CancelProcess(True, cb))
            Return False
        WinActivate, ahk_id %id%
        Send, {;}
        If(!CancelProcess(True, cb))
            Return False
        WinActivate, ahk_id %id%
        Send, {Left 2}
    }
    Else
    {
        If(!CancelProcess(True, cb))
            Return False
        WinActivate, ahk_id %id%
        Send, {Left}
    }
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {Up}
    If(str == "class")
    {
        If(g_public)
        {
            clip = public:
            If(!OutputString(g_active_id, cb, clip))
                Return
            If(g_private || g_protected)
            {
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Enter}
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, +{Home}
                If(!OutputString(g_active_id, cb, front_clip))
                    Return
            }
        }
        If(g_private)
        {
            clip = private:
            If(!OutputString(g_active_id, cb, clip))
                Return
            If(g_protected)
            {
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Enter}
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, +{Home}
                If(!OutputString(g_active_id, cb, front_clip))
                    Return
            }
        }
        If(g_protected)
        {
            clip = protected:
            If(!OutputString(g_active_id, cb, clip))
                Return
        }
        If(g_public)
        {
            If(g_private && g_protected)
            {
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Up 2}
            }
            Else If(g_private || g_protected)
            {
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Up}
            }
        }
        Else
        {
            If(g_private && g_protected)
            {
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Up}
            }
        }
        If(g_public || g_private || g_protected)
        {
            If(!CancelProcess(True, cb))
                Return
            WinActivate, ahk_id %g_active_id%
            Send, {End}
            If(!CancelProcess(True, cb))
                Return
            WinActivate, ahk_id %g_active_id%
            Send, {Enter}
            If(!CancelProcess(True, cb))
                Return
            WinActivate, ahk_id %g_active_id%
            Send, +{Home}
            If(!OutputString(g_active_id, cb, front_clip))
                Return
        }
    }
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {Tab}
    
    Return True
}

;文字列出力関数
OutputString(id, cb, str)
{
    Clipboard = %str%
    
    ;スリープ0.15秒
    Sleep, 150
    ;クリップボードにコピーした後にすぐにペーストすると、
    ;処理が速すぎて飛ばされてしまい反映されないのでスリープを入れる。
    
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, ^v
    
    Return True
}

;キー長押し処理停止関数
CancelProcess(cbflag, cb:="")
{
    ;F8長押しで停止(Sleepを考慮して最低約150ミリ秒間)
    GetKeyState, result, F8, P
    If(result == "D")
    {
        If(cbflag)
            Clipboard = %cb%
        MsgBox, 処理を停止しました。
        Return False
    }
    
    Return True
}

;IME状態の取得関数
IMEGetState()
{
    DetectHiddenWindows, On
    WinGet, current_window, ID, A
    get_default := DllCall("imm32.dll\ImmGetDefaultIMEWnd", "Uint", current_window)
    ime_state := DllCall("user32.dll\SendMessageA", "UInt", get_default, "UInt", 0x0283, "Int", 0x0005, "Int", 0)
    DetectHiddenWindows, Off
    If (ime_state)
        Send, {vkf3}
    ;IME状態が1はIMEオン(全角ひらがな)
    ;IME状態が0はIMEオフ(半角英数字)
}

#IfWinActive
