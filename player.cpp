#include "player.h"
#include <QRandomGenerator>
#include <QPropertyAnimation>

int Player::graphics_radius = 200;
int Player::graphics_x = 250;
int Player::graphics_y = 265;
int Player::score_radius = 50;
int Player::score_x = 0;
int Player::score_y = -22;

Player::Player(QWidget *parent)
    : QWidget{parent}
{
    curr_id=-1;
    angle=0;
    score=0;
    graphics_label = new QLabel(this);
    score_label = new QLabel(this);
    image = new QImage();
    connect(this,&Player::angle_changed,this,&Player::update_position);
}

void Player::init(int id, bool hard /* = false*/){
    curr_id=id;
    if (hard){
        score=0;
        score_label->setText("");
        score_label->hide();
        graphics_label->setPixmap(QPixmap::fromImage(*image));
    }
}

void Player::set_angle(double ang){
    angle = ang;
}

void Player::goto_angle(double ang){
    auto anim = new QPropertyAnimation(this,"angle",this);
    anim->setStartValue(angle);
    anim->setEndValue(ang);
    anim->setDuration(300);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void Player::update_position(double){
    move(cos(angle)*graphics_radius+graphics_x,sin(angle)*graphics_radius+graphics_y);
    score_label->move(-cos(angle)*score_radius+score_x,-sin(angle)*score_radius+score_y);
}

void Player::load_image(QString file_name/* = "" */){
    if(file_name=="") file_name=name;
    image->load(":/image/"+file_name+".png");
}

void Player::add_connection(QLineF* line,int id){//id=1:from && id=2:to
    connections.push_back(line);
    if(id==1){
        connect(this,&Player::angle_changed,[&](double){line->setP1(this->rect().center());});
    }
    else{
        connect(this,&Player::angle_changed,[&](double){line->setP2(this->rect().center());});
    }
}

void Player::clear_connection(){
    for(auto l:connections) l->setLine(0,0,0,0);
    connections.clear();
}

void Player::eliminate(){
    clear_connection();
    score_label->hide();
    auto anim = new QPropertyAnimation(graphics_label,"pos",this);
    anim->setStartValue(graphics_label->rect());
    double dis = QRandomGenerator::global()->generateDouble()*20;
    anim->setEndValue(QRectF(graphics_label->rect().topLeft()+QPointF(cos(angle)*dis,sin(angle)*dis),QSize(0,0)));
    connect(anim,&QPropertyAnimation::finished,[=](){this->hide();});
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void Player::display_score(){
    score_label->setNum(score);
    score_label->show();
}

void Player::update_score(int s){
    score+=s;
}

Player_Pair::Player_Pair(QSharedPointer<Player> in_pl1,QSharedPointer<Player> in_pl2){
    pl.resize(2);
    pl[0]=in_pl1;
    pl[0]->init(0);
    pl[1]=in_pl2;
    pl[1]->init(1);
}

Player_Pair::Player_Pair(){
    pl.resize(2);
}

QDebug operator << (QDebug debug, Player_Pair p){
    debug<<"\n"<<p[0]->name<<"\t:\t"<<p[1]->name<<"\n";
    return debug;
}

int Player::random_mistake(int choice){
    int randomNumber=QRandomGenerator::global()->bounded(1,101);
    return (randomNumber<=probility ? choice : 1-choice);
}

Player_Cooperator::Player_Cooperator(QWidget *parent) : Player(parent){
    name="cooperator";
    type=2;
    load_image();
}

QSharedPointer<Player> Player_Cooperator::clone(){
    auto tmp = QSharedPointer<Player>(new Player_Cooperator(parentWidget()));
    tmp->set_angle(angle);
    return tmp;
}

int Player_Cooperator::choice(const QList< Match_Result > & history){
    return random_mistake(0);
}

Player_Cheater::Player_Cheater(QWidget *parent) : Player(parent){
    name="cheater";
    type=1;
    load_image();
}

QSharedPointer<Player> Player_Cheater::clone(){
    auto tmp = QSharedPointer<Player>(new Player_Cheater(parentWidget()));
    tmp->set_angle(angle);
    return tmp;
}

int Player_Cheater::choice(const QList< Match_Result > & history){
    return random_mistake(1);
}

Player_Copy_Cat::Player_Copy_Cat(QWidget *parent) : Player(parent){
    name="copy cat";
    type=0;
    load_image("copycat");
}

QSharedPointer<Player> Player_Copy_Cat::clone(){
    auto tmp = QSharedPointer<Player>(new Player_Copy_Cat(parentWidget()));
    tmp->set_angle(angle);
    return tmp;
}

int Player_Copy_Cat::choice(const QList< Match_Result > & history){
    if(history.empty()) return random_mistake(0);
    return random_mistake(history.back().action[1-curr_id]);
}

Player_Random::Player_Random(QWidget *parent) : Player(parent){
    name="random";
    type=7;
}

QSharedPointer<Player> Player_Random::clone(){
    auto tmp = QSharedPointer<Player>(new Player_Random(parentWidget()));
    tmp->set_angle(angle);
    return tmp;
}

int Player_Random::choice(const QList< Match_Result > & history){
    return QRandomGenerator::global()->generate() % 2;
}

Player_Grudger::Player_Grudger(QWidget *parent) : Player(parent){
    name="grudger";
    type=3;
}

QSharedPointer<Player> Player_Grudger::clone(){
    auto tmp = QSharedPointer<Player>(new Player_Grudger(parentWidget()));
    tmp->set_angle(angle);
    return tmp;
}

int Player_Grudger::choice(const QList<Match_Result> & history){
    int flag=0;//0表示没有被cheat过，1表示被cheat过
    if(history.empty()) return random_mistake(0);
    for(auto it:history) if(it.action[1-curr_id]){flag=1;break;}
    return random_mistake(flag);
}

Player_Detective::Player_Detective(QWidget *parent) : Player(parent){
    name="detective";
    type=4;
}

QSharedPointer<Player> Player_Detective::clone(){
    auto tmp = QSharedPointer<Player>(new Player_Detective(parentWidget()));
    tmp->set_angle(angle);
    return tmp;
}

int Player_Detective::choice(const QList< Match_Result > & history){
    if(history.size()<=3)
        return random_mistake(history.size()%2);
    else{
        int flag=0;//0 for acts like copy cat , 1 for acts like cheat
        if(history[0].action[1-curr_id]+history[1].action[1-curr_id]+history[2].action[1-curr_id]+history[3].action[1-curr_id]) flag=0;
        else flag=1;
        if(flag==0) return random_mistake(history.back().action[1-curr_id]);
        else return random_mistake(1);
    }
}

Player_Copy_Kitten::Player_Copy_Kitten(QWidget *parent) : Player(parent){
    name="copy kitten";
    type=5;
}


QSharedPointer<Player> Player_Copy_Kitten::clone(){
    auto tmp = QSharedPointer<Player>(new Player_Copy_Kitten(parentWidget()));
    tmp->set_angle(angle);
    return tmp;
}

int Player_Copy_Kitten::choice(const QList< Match_Result > & history){
    if(history.size()<2) return random_mistake(0);
    int len=history.size();
    if(history[len-1].action[1-curr_id] && history[len-2].action[1-curr_id]) return random_mistake(1);
    return random_mistake(0);
}

Player_Simpleton::Player_Simpleton(QWidget *parent) : Player(parent){
    name="simpleton";
    type=6;
}

QSharedPointer<Player> Player_Simpleton::clone(){
    auto tmp = QSharedPointer<Player>(new Player_Simpleton(parentWidget()));
    tmp->set_angle(angle);
    return tmp;
}

int Player_Simpleton::choice(const QList< Match_Result > & history){
    if(history.empty()) return random_mistake(0);
    if(history.back().action[1-curr_id]) return random_mistake(1-history.back().action[curr_id]);
    else return random_mistake(history.back().action[curr_id]);
}


bool PlayerType_Compare(const Player& p1,const Player &p2){
    return p1.type<p2.type;
}
bool PlayerScore_Compare(const Player& p1,const Player &p2){
    return p1.score>p2.score;
}

bool PlayerPtrType_Compare(const QSharedPointer<Player>& p1,const QSharedPointer<Player>& p2){
    return p1->type<p2->type;
}

bool PlayerPtrScore_Compare(const QSharedPointer<Player>& p1,const QSharedPointer<Player>& p2){
    return p1->score>p2->score;
}

int& Player::get_type(){
    return type;
}
