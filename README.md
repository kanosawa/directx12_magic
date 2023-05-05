# directx12_magic

![sample](https://user-images.githubusercontent.com/13146290/236548779-d4a54dff-80ab-4d38-b336-c0d263cd2ef9.gif)

このリポジトリは[『DirectX12の魔道書（翔泳社）』](https://www.shoeisha.co.jp/book/detail/9784798161938)の非公式実装であり、公式実装の[directx12_samples](https://github.com/boxerprogrammer/directx12_samples)を基にしています。9章（リファクタリング）以前の章に対して、処理を関数に分割し、コード全体の見通しの良さを向上させています。詳しくは、[本リポジトリの長所](#本リポジトリの長所)の節を参照してください。


## 参照リポジトリ

* [directx12_samples](https://github.com/boxerprogrammer/directx12_samples) (MITライセンス)
* [DirectXTex](https://github.com/microsoft/DirectXTex) (MITライセンス)


## 使い方

Clone後、各チャプターフォルダのslnファイルをVisual Studio 2019で開き、x64/Debugモードで実行してください。それ以外の環境で実行する場合は、[書籍](https://www.shoeisha.co.jp/book/detail/9784798161938)を参照して、DirectXTex/DirectXTex.libを差し替えるなどの作業を行う必要があります。また、[公式実装](https://github.com/boxerprogrammer/directx12_samples)同様、Chapter07以降の実行にはPMDモデルファイルの用意が必要です。

## 本リポジトリの長所

* 処理が関数に分割されているため、コード全体の見通しが良い
* 各章で追加される処理が別々のcpp/hファイルに記述されているため、章ごとの相違点が分かりやすい
* グローバル変数が排除されているため、処理の流れが分かりやすい
* 変数名が分かりやすい


## 本リポジトリの短所

本リポジトリのソースコードは以下のような短所を抱えています。[公式実装](https://github.com/boxerprogrammer/directx12_samples)や[書籍](https://www.shoeisha.co.jp/book/detail/9784798161938)と合わせてご利用ください。

* 公式実装と整合がとれていない箇所がある
* 細かいコメントを削除しており、一行一行の意味は分かりにくい（全体の見通しを良くすることを優先）
* 変数名が長い、関数の引数が多い