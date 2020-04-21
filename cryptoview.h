#ifndef CRYPTOVIEW_H
#define CRYPTOVIEW_H

#include "coin.h"
#include <coin_db.h>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLabel>
#include <QMap>
#include <QList>

#include "socketclient.h"
#include "httpclient.h"
#include "candlestickdialog.h"

namespace Ui {
class CryptoView;
}

class CryptoView : public QMainWindow
{
    Q_OBJECT

public:
    explicit CryptoView(QWidget *parent = 0);
    void setCurrencyLabelText(coin& coin, QLabel& imgLabel, QLabel& valueLabel, QLabel& titleLabel );
    void pushCoin(coin& coin);
    void updatelabel(QString str);
    void formLoad();
    ~CryptoView();

private slots:
    void onCoinUpdate(int channelId, CoinData data);
    void onNewCoin(int channelId, QString pair);
    void onResponse(QNetworkReply *reply);

    void on_coin1_btn_clicked();

  /*  void on_coin2_btn_clicked();

    void on_coin3_btn_clicked();

    void on_coin4_btn_clicked(); */

private:
    Ui::CryptoView *ui;
    SocketClient *socketClient;
    HTTPClient *httpClient;
    QMap<int, QString> channelMap;
    QMap<int, QString> coinRankMap;
    QMap<QString, coin*> coinMap;
    CandleStickDialog *candles;
};

#endif // CRYPTOVIEW_H
