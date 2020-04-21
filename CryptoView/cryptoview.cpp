#include "cryptoview.h"
#include "ui_cryptoview.h"
#include "coin.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>

// Databse Path
static const QString path = "coins.db";

CryptoView::CryptoView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CryptoView)
{
    ui->setupUi(this);

    httpClient = new HTTPClient(this);
    connect(httpClient, &HTTPClient::response, this, &CryptoView::onResponse);
    httpClient->get(QUrl("https://altmarkets.io/api/v2/tickers/hthbtc"));

    socketClient = new SocketClient(this);
    connect(socketClient, &SocketClient::coinUpdate, this, &CryptoView::onCoinUpdate);
    connect(socketClient, &SocketClient::newCoin, this, &CryptoView::onNewCoin);

    //Lock Size
    this->setFixedSize(this->width(),this->height());

    // Load Currency Values from api and do all form load
    formLoad();
}

CryptoView::~CryptoView()
{
    socketClient->disconnect();
    httpClient->disconnect();
    delete ui;
}

// Helper function to rank each coin
void CryptoView::pushCoin(coin& coin){

    // Grab rank and set to variable
    int rank = coin.getRank();

    // Use switch to check what rank, and call helper function with correct labels.
    switch (rank) {
    case 1:
        setCurrencyLabelText(coin, *ui->coin1_img, *ui->coin1_value, *ui->coin1_title);
        break;
  /*  case 2:
        setCurrencyLabelText(coin, *ui->coin2_img, *ui->coin2_value, *ui->coin2_title);
        break;
    case 3:
        setCurrencyLabelText(coin, *ui->coin3_img, *ui->coin3_value, *ui->coin3_title);
        break;
    case 4:
        setCurrencyLabelText(coin, *ui->coin4_img, *ui->coin4_value, *ui->coin4_title);
        break;
    default:
        break; */
    }

    //qDebug() << "Rank is: " << rank;
}

//Helper class to update ui
void CryptoView::setCurrencyLabelText(coin& coin, QLabel& imgLabel, QLabel& valueLabel, QLabel& titleLabel ){

    //check coin trend then set colors accordingly.
    if(coin.getPercent_change_24h() > 0){
        valueLabel.setText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body style=\" font-family:'.SF NS Text'; font-size:13pt; font-weight:400; font-style:normal;\"><p align=\"center\" style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt; color:#ffffff;\">$" + QString::number(coin.getPrice_usd()) + "</span><span style=\" font-size:14pt; color:#45bf55;\"> +" +QString::number(coin.getPercent_change_24h()) +" % </span></p></body></html>");
    }else{
        valueLabel.setText("<html><head/><body><p align=\"center\"><span style=\" font-size:14pt; color:#ffffff;\">$"+ QString::number(coin.getPrice_usd()) +"<span style=\"color: #EA5455\"> " +QString::number(coin.getPercent_change_24h()) +"%    </span></span></p></body></html>");
    }

    //Set Coin title
    titleLabel.setText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body style=\" font-family:'Helvetica Neue'; font-size:14pt; font-weight:400; font-style:normal;\"><p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Helvetica Neue,Helvetica,Arial,sans-serif'; font-size:24px; font-weight:496; color:#ffffff;\">" + coin.getSymbol() + "</span></p></body></html>");

    QString abreviation = coin.getSymbol();

    QPixmap mypix (":resources/white-logos/" + abreviation.toLower() + ".png");
    imgLabel.setPixmap(mypix);

}

//Form Load
void CryptoView::formLoad()
{
    DbManager db(path);

    //Check If Db isOpen.
    if (db.isOpen())
    {
        //db.createTable();   // Creates a table if it doens't exist. Otherwise, it will use existing table.
    }
    else
    {
        qDebug() << "Database is not open!";
    }
}

// Updaing coin data
void CryptoView::onCoinUpdate(int channelId, CoinData data)
{
    // Handle coin updates
    //qDebug() << channelId << ":" << price;
    //qDebug() << data.lastPrice << "  " << data.dailyChangePercent;
    QString symbol = channelMap.value(channelId);
    coinMap.value(symbol)->setPrice_usd(data.lastPrice);
    coinMap.value(symbol)->setPercent_change_24h(data.dailyChangePercent);
    // UPDATE UI
    pushCoin(*coinMap.value(symbol));
}

// When a new coin is added add it to a map
// channelid -> coin
void CryptoView::onNewCoin(int channelId, QString pair)
{
    // Handle new coin
    QString symbol = pair.replace("USD", "");
    channelMap.insert(channelId, symbol);
}

// On httprequet data recieved
// Handle data
void CryptoView::onResponse(QNetworkReply *reply)
{
    // Read all data from the reply
    QString strReply = (QString)reply->readAll();

    // Parse JSON
    // Might be a better way of doing this
    QJsonDocument jsonDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonArray json_array = jsonDoc.array();

    QList<QString> symbols;
    foreach (const QJsonValue &value, json_array) {
        QJsonObject json_obj = value.toObject();

        symbols.push_back(json_obj["symbol"].toString());

        coin *newCoin = new coin();
        newCoin->setSymbol(json_obj["symbol"].toString());
        newCoin->setRank(json_obj["rank"].toString().toInt());
        coinMap.insert(json_obj["symbol"].toString(), newCoin);
        coinRankMap.insert(json_obj["rank"].toString().toInt(), json_obj["symbol"].toString());
    }

    // Open our websocket connection and pass top coins
    socketClient->open(QUrl("https://altmarkets.io/api/v2/tickers/hthbtc"), symbols);
}

void CryptoView::on_coin1_btn_clicked()
{
    //Show candle stick widget
    candles = new CandleStickDialog(coinRankMap.value(1), this);
    candles->show();
}


/*void CryptoView::on_coin2_btn_clicked()
{
    //Show candle stick widget
    candles = new CandleStickDialog(coinRankMap.value(2), this);
    candles->show();
}

void CryptoView::on_coin3_btn_clicked()
{
    //Show candle stick widget
    candles = new CandleStickDialog(coinRankMap.value(3), this);
    candles->show();
}

void CryptoView::on_coin4_btn_clicked()
{
    //Show candle stick widget
    candles = new CandleStickDialog(coinRankMap.value(4), this);
    candles->show();
} */
