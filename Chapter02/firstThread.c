#include <pthread.h>   // POSIXスレッド(pthread) API（スレッド生成・待機・同期など）
#include <unistd.h>    // sleep() などのPOSIX関数
#include <stdlib.h>    // exit()
#include <stdio.h>     // printf()

/*
  threadFunc: 新しく生成されるスレッドが実行する「スレッド関数」。

  ■ pthreadにおけるスレッド関数の決まりごと
  - 返り値型は void* 、引数も void* にする（汎用ポインタで受け渡しするため）
  - 実際には、arg を (int*) や (struct Foo*) にキャストして
    スレッドへ渡したいデータを受け取るのが一般的
  - 戻り値（void*）は pthread_join() で受け取れる（スレッドの終了コードのように使える）
*/
void *threadFunc(void *arg) {
    int i;

    /*
      ■ このスレッドの処理（アルゴリズムの流れ）
      1) i = 0..2 の3回ループする
      2) そのたびにメッセージを出力する
      3) sleep(1) で 1秒休む（実行の間隔を作る）

      ■ 注意: 出力の順序について
      mainスレッドと threadFuncスレッドが同時に動くので、
      printf の表示順は「実行スケジューラ次第」で前後しうる。
      ただし、両方とも sleep(1) を入れているため、
      だいたい交互っぽく見えることが多い（保証ではない）。
    */
    for(i = 0; i < 3; i++) {
        printf("I'm threadFunc: %d\n", i);

        /*
          sleep(1): 呼び出したスレッドを最低1秒間休止させる。
          - ここでは「並行実行している感じ」を見やすくするために入れている。
          - CPUを使ってビジーループするのと違って、待機中はCPU負荷が下がる。
        */
        sleep(1);
    }

    /*
      ■ スレッド終了
      return NULL; は pthread_exit(NULL); とほぼ同義（終了時にNULLを返す）。

      ※ 重要:
      スレッドが確実に終了したことを main 側で保証したいなら、
      main は pthread_join() で待つ必要がある。
      （このコードは join していないため、main が先に終わると
       プロセスが終了してスレッドも強制的に終わる可能性がある）
    */
    return NULL;
}

int main(void) {
    pthread_t thread;  // pthread_create() で生成されたスレッドを識別するためのハンドル
    int i;

    /*
      pthread_create(&thread, attr, start_routine, arg)

      ■ 引数の意味
      - &thread: 作成したスレッドID（pthread_t）を受け取る出力先
      - NULL: スレッド属性（スタックサイズやdetach状態など）をデフォルトにする
      - threadFunc: 新スレッドが実行する関数（開始点）
      - NULL: threadFunc に渡す引数（void*）。今回は渡すデータがないのでNULL

      ■ 成功/失敗
      - 成功: 0
      - 失敗: 0以外（エラー番号相当）
    */
    if(pthread_create(&thread, NULL, threadFunc, NULL) != 0) {
        printf("Error: Failed to create new thread.\n");
        exit(1);
    }

    /*
      ■ mainスレッド側の処理（アルゴリズムの流れ）
      1) i = 0..4 の5回ループする
      2) そのたびにメッセージを出力する
      3) sleep(1) で 1秒休む

      この間も、別スレッド(threadFunc)が並行して走っている。
      つまり、プロセス内に「mainスレッド」と「threadFuncスレッド」が共存する。
    */
    for(i = 0; i < 5; i++) {
        printf("I'm main: %d\n", i);
        sleep(1);
    }

    /*
      ■ このコードの重要なポイント（同期・終了の話）
      - ここで main が return 0; すると「プロセス」が終了する。
        プロセスが終了すると、その中の全スレッドも終了する。
      - 今回は main が5秒程度動き、threadFunc は3秒程度で終わるので、
        たまたま threadFunc が先に終わりやすく、問題が表面化しにくい。

      ■ でも一般には、スレッド終了を保証するために pthread_join() を書くべき
        pthread_join(thread, NULL);
      これを書くと main は threadFunc の終了まで待つ（同期）。

      ※ joinしない場合の典型的な問題:
      - main が先に終わってスレッドが途中で打ち切られる
      - スレッドの終了リソース（いわゆる joinable なスレッドの後始末）が回収されない
        （短命プロセスなら問題になりにくいが、長時間動くプログラムでは積み重なる）
    */
    return 0;
}
