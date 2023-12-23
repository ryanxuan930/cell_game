#include <iostream>
#include <vector>
#include <limits>
#include <cstdlib>
#include <thread>
#include <chrono>
using namespace std;

//0 = dead, 1=survive
int ROWS, COLUMNS;
int MAX_GENERATIONS;
int numbers_1,numbers_2,numbers_3,numbers_4,numbers_5,numbers_6,numbers_7,numbers_8,numbers_9,numbers;
int count_eater;
vector<int> NiceNeighbor;  //**增加
int seeds;
float seconds;
vector<vector<vector<int>>> position;//地圖，用3D向量(vector)存
vector<vector<int>> terrain;//地形
/*額外規則介紹
一.地形
0:一般 1:水域 2:沙漠 3:岩石
水域:資源足夠，因此維持生命所需減少，只要1個細胞在周圍即可存活且就算周邊有四個細胞也能存活 2個即可繁殖
沙漠，荒地:資源缺乏，因此生存條件變坎坷，周圍只能剛好兩個細胞才能存活，且若連續在沙漠上三個世代會死亡 4個才能繁殖
岩石:障礙物，阻擋細胞
二.細胞總類
0:空格 1:普通細胞 2:水域細胞 3:沙漠細胞 4:掠食者 5:寄生普通細胞 6:寄生水域細胞 7:寄生沙漠細胞 8:大掠食者 9:普通大細胞
除了空格與掠食者以外，皆可符合為"周遭的細胞"條件
水域細胞與沙漠細胞若在對方的生態域會難以生存
(各種能適應不同地形的細胞)
掠食者:會吃掉周圍的普通細胞，每隔三個世代會吃一次，只要沒東西吃即死亡，若周圍有一個掠食者也死亡
寄生:會寄生於普通細胞上，使細胞生存條件變嚴峻，周圍只能剛好兩個細胞才能存活，若在沙漠上，則活兩個世代後即死亡
三.進化型態
一般細胞:連續活三個世代後會變顏色且免疫掠食者與寄生，若在沙漠上變黃色且使生存條件變回一般狀態，若在水域中，則變藍色且就算沒有細胞在周圍也可存活
掠食者:吃完三次後，則改為每五個世代吃一次
寄生:寄生同一個宿主連續三個世代後會往周圍進行寄生擴散
四.天氣
(每十天改變地形)
區域暴雨:增加水域
區域旱災:增加沙漠
*/

/* function介紹
detect:偵測周遭細胞存活數紀錄在count裡
updateCellStatus:利用detect後的count判斷現在細胞應該是甚麼狀態
customizePattern:初始化
customizeMapSize:使用者自定邊界
show:圖像化
信號燈、方塊、桶子、滑翔機皆已成功
*/

/*演算法介紹
先用customizeMapSize函式設定地圖大小，接著利用customizePattern接受輸入初始細胞位置
之後利用detect函式判斷當下該細胞附近有多少細胞
得到結果後用updateCellStatus函式判斷下一秒該細胞的狀態並輸入進下一秒
最後利用show函式輸出圖像
*/
//===================================================================



















//===================================================================
int detect(int row,int column,int time){
    int count = 0;
    count_eater=0;
    for (int i = max(0, row - 1); i <= min(ROWS - 1, row + 1); i++) {  //max&min用來當作邊界偵測
        for (int j = max(0, column - 1); j <= min(COLUMNS - 1, column + 1); j++) {
            if (i == row && j == column) {
                continue;  // 不包括當前细胞本身
            }
            if (position[time][i][j]!=0&&position[time][i][j]!=4&&position[time][i][j]!=8) {
                count++;
            }
            if(position[time][i][j]==4||position[time][i][j]==8){
                count_eater++;
            }
			if(position[time][i][j]==9 || position[time][i][j]==3){  //**增加
				NiceNeighbor[time]++;
			}
        }
    }
    return count;
}

void customizeMapSize() {
    std::cout << "請輸入地圖的列數(row): ";
    std::cin >> ROWS;
    while (std::cin.fail() || ROWS <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "無效輸入，請輸入正整數：";
        std::cin >> ROWS;
    }
    std::cout << "請輸入地圖的行數(column): ";
    std::cin >> COLUMNS;
    while (std::cin.fail() || COLUMNS <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "無效輸入，請輸入正整數：";
        std::cin >> COLUMNS;
    }
    std::cout << "請輸入想要觀察的總秒數: ";
    std::cin >> MAX_GENERATIONS;
    while (std::cin.fail() || MAX_GENERATIONS <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "無效輸入，請輸入正整數：";
        std::cin >> MAX_GENERATIONS;
    }
    std::cout << "請輸入想要每一秒的間隔有多長(幾秒): ";
    std::cin >> seconds;
    while (std::cin.fail() || seconds <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "無效輸入，請輸入正實數：";
        std::cin >> seconds;
    }
    position = vector<vector<vector<int>>>(MAX_GENERATIONS, vector<vector<int>>(ROWS, vector<int>(COLUMNS, 0)));
    terrain = vector<vector<int>>(ROWS, vector<int>(vector<int>(COLUMNS, 0)));
    NiceNeighbor = vector<int>(MAX_GENERATIONS);
}

void updateCellStatus(int row,int column,int time){
    if(time!=(MAX_GENERATIONS-1)){
        if(terrain[row][column]==0){                 //一般地形
            if(position[time][row][column] == 0){          //當前細胞為死亡狀態時，當周圍有3個存活細胞時，該細胞變成存活狀態。
                if(detect(row,column,time) == 3){
                    position[time+1][row][column] = 1;
                }
                else{
                    position[time+1][row][column] = 0;
                }
            }
            else if(position[time][row][column] == 1){     //當前細胞為存活狀態時，當周圍的存活細胞低於2個或大於3個時，該細胞變成死亡狀態。
                if(detect(row,column,time) < 2||detect(row,column,time) > 3){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 1;
                }
            }
            else if(position[time][row][column] == 2){     //當前細胞為存活狀態時，當周圍的存活細胞低於2個或大於3個時，該細胞變成死亡狀態。
                if(detect(row,column,time) < 2||detect(row,column,time) > 3){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 2;
                }
            }
            else if(position[time][row][column] == 3){     //當前細胞為存活狀態時，當周圍的存活細胞低於2個或大於3個時，該細胞變成死亡狀態。
                if(detect(row,column,time) < 2||detect(row,column,time) > 3){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 3;
                }
            }
            else if(position[time][row][column] == 4){
                if(count_eater>=1){
                    position[time+1][row][column]= 0;
                }
                else{
                    position[time+1][row][column] = 4;
                }
            }
            else if(position[time][row][column] == 5){
                position[time+1][row][column] = (detect(row,column,time)==2)?5:0;
            }
            else if(position[time][row][column] == 6){
                position[time+1][row][column] = (detect(row,column,time)==2)?6:0;
            }
            else if(position[time][row][column] == 7){
                position[time+1][row][column] = (detect(row,column,time)==2)?7:0;
            }
            else if(position[time][row][column] == 8){
                if(count_eater>=2){
                    position[time+1][row][column]= 0;
                }
                else{
                    position[time+1][row][column] = 8;
                }
            }
            else if(position[time][row][column] == 9){
                if(detect(row,column,time) < 2||detect(row,column,time) > 3){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 9;
                }
            }
        }
        if(terrain[row][column]==1){                 //水域地形
            if(position[time][row][column] == 0){          //當前細胞為死亡狀態時，當周圍有3個存活細胞時，該細胞變成存活狀態。
                if(detect(row,column,time) == 2){
                    position[time+1][row][column] = 1;
                }
                else{
                    position[time+1][row][column] = 0;
                }
            }
            else if(position[time][row][column] == 1){     //當前細胞為存活狀態時，當周圍的存活細胞低於2個或大於3個時，該細胞變成死亡狀態。
                if(detect(row,column,time) < 1||detect(row,column,time) > 4){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 1;
                }
            }
            else if(position[time][row][column] == 2){     //當前細胞為存活狀態時，當周圍的存活細胞低於2個或大於3個時，該細胞變成死亡狀態。
                if(detect(row,column,time) > 4){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 2;
                }
            }
            else if(position[time][row][column] == 3){     //當前細胞為存活狀態時，當周圍的存活細胞低於2個或大於3個時，該細胞變成死亡狀態。
                if(detect(row,column,time) !=3){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 3;
                }
            }
            else if(position[time][row][column] == 4){
                if(count_eater>=1){
                    position[time+1][row][column]= 0;
                }
                else{
                    position[time+1][row][column] = 4;
                }
            }
            }
            else if(position[time][row][column] == 5){
                position[time+1][row][column] = (detect(row,column,time)==2)?5:0;
            }
            else if(position[time][row][column] == 6){
                position[time+1][row][column] = (detect(row,column,time)==2)?6:0;
            }
            else if(position[time][row][column] == 7){
                position[time+1][row][column] = (detect(row,column,time)==2)?7:0;
            }
            else if(position[time][row][column] == 8){
                if(count_eater>=2){
                    position[time+1][row][column]= 0;
                }
                else{
                    position[time+1][row][column] = 8;
                }
            }
            else if(position[time][row][column] == 9){
                if(detect(row,column,time) < 1||detect(row,column,time) > 4){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 9;
                }
            }
        }
        if(terrain[row][column]==2){                 //沙漠地形
            if(position[time][row][column] == 0){     //**更改規則**
                if(detect(row,column,time) == 4){
                    position[time+1][row][column] = 1;
                }
                else{
                    position[time+1][row][column] = 0;
                }
            }
            else if(position[time][row][column] == 1){     //**改規則***
                if(detect(row,column,time) != 2){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 1;
                    if(time>=2){
                        if(position[time-2][row][column]==1&&position[time-1][row][column]==1&&position[time][row][column]==1){
                            if(NiceNeighbor[time-2] !=0 || NiceNeighbor[time-1] !=0  || NiceNeighbor[time] !=0)
								position[time+1][row][column] =3;	//進化成沙漠細胞
							else
								position[time+1][row][column] =0;
                        }
                    }
                }
            }
            else if(position[time][row][column] == 2){     //當前細胞為存活狀態時，當周圍的存活細胞低於2個或大於3個時，該細胞變成死亡狀態。
                if(detect(row,column,time) != 3){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 2;
                    if(time>=2){
                        if(position[time-2][row][column]==2&&position[time-1][row][column]==2&&position[time][row][column]==2){
                            position[time+1][row][column] =0;
                        }
                    }
                }
            }
            else if(position[time][row][column] == 3){     //當前細胞為存活狀態時，當周圍的存活細胞低於2個或大於3個時，該細胞變成死亡狀態。
                if(detect(row,column,time) < 2||detect(row,column,time) > 3){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 1;
                }
            }
            else if(position[time][row][column] == 4){
                if(count_eater>=1){
                    position[time+1][row][column]= 0;
                }
                else{
                    position[time+1][row][column] = 4;
                }

                if(time>=2){
                        if(position[time-2][row][column]==4&&position[time-1][row][column]==4&&position[time][row][column]==4){
                            position[time+1][row][column] =0;
                        }
                    }
            }
            else if(position[time][row][column] == 5){
                position[time+1][row][column] = (detect(row,column,time)==2)?5:0;
                if(time>=1){                                       //進化
                    if(position[time-2][row][column]==5&&position[time-1][row][column]==5&&position[time][row][column]==5){
                        position[time+1][row][column] =0;
                    }
                }
            }
            else if(position[time][row][column] == 6){
                position[time+1][row][column] = (detect(row,column,time)==2)?6:0;
                if(time>=1){                                       //進化
                    if(position[time-2][row][column]==6&&position[time-1][row][column]==6&&position[time][row][column]==6){
                        position[time+1][row][column] =0;
                    }
                }
            }
            else if(position[time][row][column] == 7){
                position[time+1][row][column] = (detect(row,column,time)==2)?7:0;
                if(time>=1){
                    if(position[time-2][row][column]==7&&position[time-1][row][column]==7&&position[time][row][column]==7){
                        position[time+1][row][column] =0;
                    }
                }
            }
            else if(position[time][row][column] == 8){
                if(count_eater>=2){
                    position[time+1][row][column]= 0;
                }
                else{
                    position[time+1][row][column] = 8;
                }
                if(time>=2){
                    if(position[time-2][row][column]==8&&position[time-1][row][column]==8&&position[time][row][column]==8){
                        position[time+1][row][column] =0;
                    }
                }
            }
            else if(position[time][row][column] == 9){
                if(detect(row,column,time) != 2){
                    position[time+1][row][column] = 0;
                }
                else{
                    position[time+1][row][column] = 1;
                    if(time>=2){
                        if(position[time-2][row][column]==9&&position[time-1][row][column]==9&&position[time][row][column]==9){
                            position[time+1][row][column] =0;
                    }
                }
                }
            }
        }
        if (position[time][row][column] == 1){

        if(position[time][row][column]==8&&time%4==0){		//大掠食者吃細胞
            bool eaterdead=0;
            for (int i = max(0, row - 1); i <= min(ROWS - 1, row + 1); i++) {
                    for (int j = max(0, column - 1); j <= min(COLUMNS - 1, column + 1); j++) {
                            if (i == row && j == column) {
                                continue;
                            }
                            if (position[time][row][column] == 1){
                                position[time+1][row][column] =0;
                                eaterdead=1;
                            }
                    }
            }
            position[time+1][row][column]=(eaterdead)?8:0;
        }
//--------------------------------------------------------------------------------------------------------------------------------------
        if(time>=2){                                       //進化			//********
            if(position[time-2][row][column]==1&&position[time-1][row][column]==1&&position[time][row][column]==1){
                    if(terrain[row][column]==0){
                            position[time+1][row][column]=9;
                    }
                    if(terrain[row][column]==1){
                            position[time+1][row][column]=2;
                    }
                    /*if(terrain[row][column]==2){  //**刪掉
                            position[time+1][row][column]=3;
                    }*/
            }
            if((position[time-2][row][column]==5)&&(position[time-1][row][column]==5)&&(position[time][row][column]==5)){
                    for (int i = max(0, row - 1); i <= min(ROWS - 1, row + 1); i++) {
                            for (int j = max(0, column - 1); j <= min(COLUMNS - 1, column + 1); j++) {
								if(position[time][i][j] == 1)  //##############
									position[time+1][row][column]=5;
                            }
                    }
            }
			if((position[time-2][row][column]==6)&&(position[time-1][row][column]==6)&&(position[time][row][column]==6)){
                    for (int i = max(0, row - 1); i <= min(ROWS - 1, row + 1); i++) {
                            for (int j = max(0, column - 1); j <= min(COLUMNS - 1, column + 1); j++) {
								if(position[time][i][j] == 2)  //##############
									position[time+1][row][column]=6;
                            }
                    }
            }
			if((position[time-2][row][column]==7)&&(position[time-1][row][column]==7)&&(position[time][row][column]==7)){
                    for (int i = max(0, row - 1); i <= min(ROWS - 1, row + 1); i++) {
                            for (int j = max(0, column - 1); j <= min(COLUMNS - 1, column + 1); j++) {
								if(position[time][i][j] == 3)  //##############
									position[time+1][row][column]=7;
                            }
                    }
            }
        }
//--------------------------------------------------------------------------------------------------------------------------------------
        if(time%4==0&&time>=9){
            if(position[time-9][row][column]==4&&position[time-4][row][column]==4&&position[time][row][column]==4){
                     position[time+1][row][column]=8;
            }
        }
        if(terrain[row][column]==3){
            position[time+1][row][column]=0;
        }
    }
void customizePattern() {
    while(1){
        int modeChoice;
        cout << "隨機輸入還是自訂輸入呢 (隨機輸入1，自訂輸入2): ";
        cin >> modeChoice;
        if (modeChoice==1){
            int seed;
            cout<<"請輸入隨機種子碼: ";
            cin>>seed;
            seeds = seed ;
            srand(seed);
            for(int i = 0;i<ROWS;i++){
                for(int j = 0;j<COLUMNS;j++){
                        position[0][i][j]=rand()%10;
                }
            }
            break;
        }
        else if (modeChoice == 2) {
            while (1) {
                cout << "有要設定初始細胞狀態嗎? 請輸入o(新增)或x(刪除)，或輸入R(展示): ";
                char o_or_x;
                cin >> o_or_x;
                if (o_or_x == 'o' || o_or_x == 'x') {
                    int row, column,cell_type;
                    cout << "請輸入要設定的細胞座標及細胞種類 (row column皆從1開始): "<<endl<<"(0:空格 1:普通細胞 2:水域細胞 3:沙漠細胞 4:掠食者 5:寄生普通細胞 6:寄生水域細胞 7:寄生沙漠細胞 8:大掠食者 9:普通大細胞):";
                    cin >> row >> column>>cell_type;
                    if (row >= 1 && row <= ROWS && column >= 1 && column <= COLUMNS &&cell_type>=0&&cell_type<=9) {
                        position[0][row - 1][column - 1] = cell_type;
                            for (int i = 0; i < ROWS; i++) {
                                for (int j = 0; j < COLUMNS; j++) {
                                    cout << position[0][i][j];
                                }
                            cout << endl;
                            }
                    }
                    else {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "無效的座標或種類，請重試。" << endl;
                            continue;
                        }
                }
                else if (o_or_x == 'R' || o_or_x == 'r') {
                    break;
                }
                else {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "無效輸入，請重新輸入。" << endl;
                }
            }
            break;
        }
        else {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "無效的模式，請重試。" << endl;
        }
    }
}

void updateTerrain(int time){
    if((time!=0)&&(time%10==0)){
        srand(seeds);
        int random_rows ;
        int random_columns;
        int random_terrain;
        seeds=(rand()+5)%10000;
        random_terrain = rand()%3;
        for(int k = 0;k<5;k++){
            random_rows=rand()%ROWS;
            random_columns = rand()%COLUMNS;
            for (int i = max(0, random_rows - 1); i <= min(ROWS - 1, random_rows + 1); i++) {  //max&min用來當作邊界偵測
                for (int j = max(0, random_columns - 1); j <= min(COLUMNS - 1, random_columns + 1); j++) {
                    if(terrain[i][j]==3){
                        continue;
                    }
                    terrain[i][j]=random_terrain;
                }
            }

        }
        if(random_terrain==0){
            cout<<"回歸自然，下一回合一般格子增加";
        }
        else if(random_terrain==1){
            cout<<"暴雨來襲，下一回合水域格子增加";
        }
        else if(random_terrain==2){
            cout<<"太陽直射，下一回合沙漠格子增加";
        }
    }
}

void show(){
    for(int time = 0 ;time<MAX_GENERATIONS;time++){
        cout<<"第"<<time+1<<"秒"<<endl;
        numbers_1,numbers_2,numbers_3,numbers_4,numbers_5,numbers_6,numbers_7,numbers_8,numbers_9,numbers = 0;
        cout<<"position:"<<endl;
        for(int i = 0;i < ROWS;i++){
            for(int j = 0;j < COLUMNS;j++){
                if(position[time][i][j]==0){
                    cout<<" ";                //可更改cout來換空格的表示法
                }
                else if(position[time][i][j]==1){
                    numbers_1++;
                    cout<<"*";                //可更改cout來換細胞的表示法
                }
                else if(position[time][i][j]==2){
                    numbers_2++;
                    cout<<"2";                //可更改cout來換細胞的表示法
                }
                else if(position[time][i][j]==3){
                    numbers_3++;
                    cout<<"3";                //可更改cout來換細胞的表示法
                }
                else if(position[time][i][j]==4){
                    numbers_4++;
                    cout<<"4";                //可更改cout來換細胞的表示法
                }
                else if(position[time][i][j]==5){
                    numbers_5++;
                    cout<<"5";                //可更改cout來換細胞的表示法
                }
                else if(position[time][i][j]==6){
                    numbers_6++;
                    cout<<"6";                //可更改cout來換細胞的表示法
                }
                else if(position[time][i][j]==7){
                    numbers_7++;
                    cout<<"7";                //可更改cout來換細胞的表示法
                }
                else if(position[time][i][j]==8){
                    numbers_8++;
                    cout<<"8";                //可更改cout來換細胞的表示法
                }
                else if(position[time][i][j]==9){
                    numbers_9++;
                    cout<<"9";                //可更改cout來換細胞的表示法
                }
                updateCellStatus(i,j,time);
            }
            cout<<endl;
        }
        cout<<"terrain:"<<endl;
        for(int i = 0;i < ROWS;i++){
            for(int j = 0;j < COLUMNS;j++){
                if(terrain[i][j]==0){
                    cout<<" ";
                }
                else if(terrain[i][j]==1){
                    cout<<"1";                //可更改cout來換細胞的表示法
                }
                else if(terrain[i][j]==2){
                    cout<<"2";                //可更改cout來換細胞的表示法
                }
                else if(terrain[i][j]==3){
                    cout<<"3";                //可更改cout來換細胞的表示法
                }
            }
            cout<<endl;
        }
        updateTerrain(time);
        cout<<endl;
        cout<<"總共"<<numbers+numbers_1+numbers_2+numbers_3+numbers_4+numbers_5+numbers_6+numbers_7+numbers_8+numbers_9<<"個細胞"<<endl;
        cout<<"總共"<<numbers_1<<"個一般細胞"<<endl;
        numbers_1=0;
        cout<<"總共"<<numbers_2<<"個水域細胞"<<endl;
        numbers_2=0;
        cout<<"總共"<<numbers_3<<"個沙漠細胞"<<endl;
        numbers_3=0;
        cout<<"總共"<<numbers_4<<"個掠食者細胞"<<endl;
        numbers_4=0;
        cout<<"總共"<<numbers_5<<"個寄生一般細胞"<<endl;
        numbers_5=0;
        cout<<"總共"<<numbers_6<<"個寄生水域細胞"<<endl;
        numbers_6=0;
        cout<<"總共"<<numbers_7<<"個寄生沙漠細胞"<<endl;
        numbers_7=0;
        cout<<"總共"<<numbers_8<<"個大掠食者細胞"<<endl;
        numbers_8=0;
        cout<<"總共"<<numbers_9<<"個一般大細胞"<<endl;
        numbers_9=0;

        chrono::seconds sleep_duration(1);
        this_thread::sleep_for(sleep_duration*seconds);
        for(int i = 0;i<COLUMNS+5;i++){
            cout<<"-";
        }
        cout<<endl;

    }
}

int main()
{
    customizeMapSize();
    customizePattern();
    show();
    return 0;
}
