# Effect (VFX を除く)

## パーティクル
あくまで賑やかし,メインとなるシルエットとは別ものとして考える.
### primitive
モデルを使わず,コードで形状を作る.

| 形状 | 情報 | 何処で使われる |
| ---- |

   板ポリ(横幅,縦幅,uvMin,uvMax) -> パーティクル
リング (リングの幅,内径,外径) -> シルエット
球(中心座標,スケール)          -> パーティクル
円柱(長さ,上半径,下半径) ->シルエット


などが ある.
これらに uvスクロールなどを組み合わせてエフェクトを作る.