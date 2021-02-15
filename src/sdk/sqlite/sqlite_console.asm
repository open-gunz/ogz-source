

sqlSource  text 'select ?1 as source, ?2 as ticket, ?3 as result'

proc SQLiteConsole, .pSpecial
.stmt dd ?
.source dd ?
.ticket dd ?
.result dd ?
begin
        pushad

        xor     eax, eax
        mov     [.ticket], eax
        mov     [.source], eax
        mov     [.result], eax

        stdcall TextCreate, sizeof.TText
        mov     edi, eax

        mov     esi, [.pSpecial]

        test    [esi+TSpecialParams.userStatus], permAdmin
        jz      .for_admins_only

        stdcall LogUserActivity, esi, uaAdminThings, 0

        stdcall GetPostString, [esi+TSpecialParams.post_array], "source", 0
        test    eax, eax
        jz      .source_rdy
        stdcall StrClipSpacesR, eax
        stdcall StrClipSpacesL, eax
.source_rdy:
        mov     [.source], eax

        stdcall SetUniqueTicket, [esi+TSpecialParams.session]
        jc      .for_admins_only

        mov     [.ticket], eax

        mov     eax, [esi+TSpecialParams.userLang]
        stdcall StrCat, [esi+TSpecialParams.page_title], [cSQLiteConsoleTitle+8*eax]

; first output the form

        lea     eax, [.stmt]
        cinvoke sqlitePrepare_v2, [hMainDatabase], sqlSource, -1, eax, 0

        cmp     [.source], 0
        je      .source_ok

        stdcall StrPtr, [.source]
        cinvoke sqliteBindText, [.stmt], 1, eax, [eax+string.len], SQLITE_STATIC

.source_ok:

        stdcall StrPtr, [.ticket]
        cinvoke sqliteBindText, [.stmt], 2, eax, [eax+string.len], SQLITE_STATIC

; Here execute the source query!

        stdcall GetPostString, [esi+TSpecialParams.post_array], "ticket", 0
        test    eax, eax
        jz      .result_ok

        mov     ebx, eax
        stdcall CheckTicket, ebx, [esi+TSpecialParams.session]
        pushf
        stdcall ClearTicket3, ebx
        stdcall StrDel, ebx
        popf
        jc      .result_ok

        cmp     [.source], 0
        je      .result_ok

        stdcall ExecSourceSQL, [.source]
        mov     [.result], edx

        cinvoke sqliteBindText, [.stmt], 3, edx, eax, SQLITE_STATIC

.result_ok:

        cinvoke sqliteStep, [.stmt]

.make_the_form:

        stdcall RenderTemplate, edi, "form_sqlite_console.tpl", [.stmt], [.pSpecial]
        mov     edi, eax

        cinvoke sqliteFinalize, [.stmt]
        clc

.exit:
        pushf
        stdcall TextFree, [.result]
        stdcall StrDel, [.ticket]
        stdcall StrDel, [.source]
        popf

        mov     [esp+4*regEAX], edi
        popad
        return


.for_admins_only:
        stdcall TextMakeRedirect, edi, "/!message/only_for_admins"
        stc
        jmp     .exit

endp



proc ExecSourceSQL, .hSource
.stmt  dd ?
.next  dd ?
.start dd ?
.count dd ?
begin
        pushad

        stdcall TextCreate, sizeof.TText
        mov     edi, eax

; here execute the source.

        stdcall TextCat, edi, '<div class="sql_exec">'
        mov     edi, edx

        stdcall StrPtr, [.hSource]
        mov     esi, eax

.sql_loop:
        cmp     byte [esi], 0
        je      .finish_exec

        stdcall GetFineTimestamp
        mov     [.start],eax

        lea     ecx, [.stmt]
        lea     eax, [.next]
        cinvoke sqlitePrepare_v2, [hMainDatabase], esi, -1, ecx, eax
        test    eax, eax
        jnz     .error

        stdcall StrNew

        mov     edx, [.next]
        sub     edx, esi
        stdcall StrCatMem, eax, esi, edx
        push    eax

        stdcall StrEncodeHTML, eax
        stdcall StrDel ; from the stack
        stdcall StrClipSpacesR, eax
        stdcall StrClipSpacesL, eax

        stdcall TextCat, edi, "<h5>Statement executed:</h5><pre>"
        stdcall TextCat, edx, eax
        stdcall StrDel, eax
        stdcall TextCat, edx, "</pre>"
        mov     edi, edx

; first step
        cinvoke sqliteStep, [.stmt]
        cmp     eax, SQLITE_ROW
        je      .fetch_rows

.done:
        cmp     eax, SQLITE_DONE
        jne     .error

.finalize_one:
        cinvoke sqliteFinalize, [.stmt]

        stdcall GetFineTimestamp
        sub     eax, [.start]

        stdcall NumToStr, eax, ntsDec or ntsUnsigned
        push    eax eax

        stdcall TextCat, edi, "<p>Execution time: "
        stdcall TextCat, edx ; from the stack
        stdcall StrDel ; from the stack
        stdcall TextCat, edx, txt "μs</p>"
        mov     edi, edx

        xchg    esi, [.next]
        cmp     esi, [.next]
        jne     .sql_loop

.finish_exec:
        stdcall TextCat, edi, '</div>'

        stdcall TextCompact, edx

        mov     [esp+4*regEAX], eax
        mov     [esp+4*regEDX], edx
        popad
        return


.error:
        cinvoke sqliteDBMutex, [hMainDatabase]
        cinvoke sqliteMutexEnter, eax

        cinvoke sqliteErrMsg, [hMainDatabase]

        stdcall TextCat, edi, '<p class="result_msg">'
        stdcall TextCat, edx, eax
        stdcall TextCat, edx, txt '</p>'
        mov     edi, edx

        cinvoke sqliteDBMutex, [hMainDatabase]
        cinvoke sqliteMutexLeave, eax
        jmp     .finalize_one


.fetch_rows:

; first the table

        stdcall TextCat, edi, '<table class="sql_rows"><thead><tr>'
        mov     edi, edx

        cinvoke sqliteColumnCount, [.stmt]
        mov     [.count], eax

        xor     ebx, ebx

.col_loop:
        cmp     ebx, [.count]
        jae     .end_columns

        cinvoke sqliteColumnName, [.stmt], ebx

        stdcall StrEncodeHTML, eax

        stdcall TextCat, edi, txt "<th>"
        stdcall TextCat, edx, eax
        stdcall StrDel, eax
        stdcall TextCat, edx, txt "</th>"
        mov     edi, edx

        inc     ebx
        jmp     .col_loop

.end_columns:

        stdcall TextCat, edi, txt "</tr></thead>"
        mov     edi, edx

.row_loop:

        stdcall TextCat, edi, txt "<tr>"
        mov     edi, edx

        xor     ebx, ebx

.val_loop:
        cmp     ebx, [.count]
        jae     .end_vals

        cinvoke sqliteColumnText, [.stmt], ebx
        test    eax, eax
        jnz     .txt_ok

        mov     eax, .cNULL

.txt_ok:
        stdcall StrEncodeHTML, eax
        stdcall TextCat, edi, txt "<td>"
        stdcall TextCat, edx, eax
        stdcall StrDel, eax
        stdcall TextCat, edx, txt "</td>"
        mov     edi, edx

        inc     ebx
        jmp     .val_loop

.end_vals:
        stdcall TextCat, edi, txt "</tr>"
        mov     edi, edx

        cinvoke sqliteStep, [.stmt]

        cmp     eax, SQLITE_ROW
        je      .row_loop

        stdcall TextCat, edi, "</table>"
        mov     edi, edx
        jmp     .done           ; done or error?


.cNULL db "NULL", 0
endp