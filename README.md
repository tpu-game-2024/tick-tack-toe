# 課題：強くして
本リポジトリのAIに対して、何らかの工夫をしてください！

* 計算を効率化する
* 新たなアルゴリズムを実装する

5目並べにすると、わかりやすいかもしれません。

![結果画像](image.png)

# 工夫点

* MiniMax法を使用してAIを強くしました。
すべてのマスを探索して、その結果を得点として、最適なマスを選んでコマを起きます。
そのため、プレイヤーが勝ちそうなときは、プレイヤーを妨害するように、AIが勝つときはAIが活用にしてコマを起きます。

# 取り組み方
* 本プロジェクトをforkして、取り組んでください。
* GitHub Actions (Actionsのタブ)を機能させて、README.mdに記述された下記のバッチの「tpu-game-2024」を自分のアカウントに差し替えてください。
* readme.md に実施した工夫を記載してください
* 可能であれば、速度等を計測して、具体的な効率化度合い、強さを示してください。
* 納得できるところまでできたところでプルリクを出してください。

[![MSBuild](https://github.com/Yoshizawa-TPU/tick-tack-toe/actions/workflows/msbuild.yml/badge.svg)](https://github.com/Yoshizawa-TPU/tick-tack-toe/actions/workflows/msbuild.yml)

（↑のソースコードの「tpu-game-2024」を自分のアカウント名に差し替えてください（２か所））
