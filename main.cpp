#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main( int argc, const char** argv)
{
    // Criando a variável da câmera
    VideoCapture cam(0);

    // Verifica se a câmera está conectada
    if(!cam.isOpened()){
        cout<<"ERRO: CÂMERA NÃO CONECTADA!"<< endl;
        return -1;
    }

    // Declando as variáveis das imagens
    Mat img;
    Mat img_threshold;
    Mat img_gray;
    Mat img_roi;
    Mat img_roi2;
    Mat img_gray2;
    Mat img_threshold2;

    // Criar as janelas para mostrar as imagens da câmera
    namedWindow("Original_image",CV_WINDOW_AUTOSIZE);
    namedWindow("Gray_image",CV_WINDOW_AUTOSIZE);
    namedWindow("Thresholded_image",CV_WINDOW_AUTOSIZE);
    namedWindow("Thresholded_image 2",CV_WINDOW_AUTOSIZE);
    namedWindow("ROI",CV_WINDOW_AUTOSIZE);
    namedWindow("ROI 2",CV_WINDOW_AUTOSIZE);

    // Caracteres utilizados para escrever posteriormente no frame
    char a[40];
    char c[60];

    // Contadores de dedos
    int count =0;
    int count2 =0;
    int countResultado =0;

    while(1){
        // Ler a imagem
        bool b=cam.read(img);

        // Caso não consiga ler a imagem
        if(!b){
            cout<<"ERRO: CÂMERA NÃO CONECTADA!"<<endl;
            return -1;
        }

        /*
        Rect roi(340,100,270,270);
        Rect roi2(140,100,270,270);
        */

        // Capturando duas pequenas partes da imagem para fazer a segmentação posterior
        Rect roi(400,100,200,200);
        Rect roi2(50,100,200,200);

        img_roi = img(roi);
        img_roi2 = img(roi2);

        // Transformando as imagens em escala de cinza
        cvtColor(img_roi,img_gray, CV_RGB2GRAY);
        cvtColor(img_roi2,img_gray2, CV_RGB2GRAY);

        // Segmentando a primeira divisão da imagem
        GaussianBlur(img_gray,img_gray, Size(19,19), 0.0, 0);
        threshold(img_gray,img_threshold, 0, 255, THRESH_BINARY_INV+THRESH_OTSU);

        // Segmentando a segunda divisão da imagem
        GaussianBlur(img_gray2, img_gray2, Size(19,19), 0.0, 0);
        threshold(img_gray2,img_threshold2,0,255,THRESH_BINARY_INV+THRESH_OTSU);

        vector<vector<Point> >contours;
        vector<vector<Point> >contours2;
        vector<Vec4i>hierarchy;

        // Pegar contornos das imagens
        findContours(img_threshold,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE,Point());
        findContours(img_threshold2,contours2,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE,Point());

        // Caso detecte algum contorno na imagem
        if(contours.size()>0){
                size_t indexOfBiggestContour = -1;
                size_t sizeOfBiggestContour = 0;

                for (size_t i = 0; i < contours.size(); i++){
                    if(contours[i].size() > sizeOfBiggestContour){
                        sizeOfBiggestContour = contours[i].size();
                        indexOfBiggestContour = i;
                    }
                }

                vector<vector<int> >hull(contours.size());
                vector<vector<Point> >hullPoint(contours.size());
                vector<vector<Vec4i> >defects(contours.size());
                vector<vector<Point> >defectPoint(contours.size());
                vector<vector<Point> >contours_poly(contours.size());
                Point2f rect_point[4];
                vector<RotatedRect>minRect(contours.size());
                vector<Rect> boundRect(contours.size());

                // Utilizando o convexo hull para contar a quantidade de dedos
                for(size_t i=0;i<contours.size();i++){
                    if(contourArea(contours[i])>5000){
                        convexHull(contours[i],hull[i],true);
                        convexityDefects(contours[i],hull[i],defects[i]);
                        if(indexOfBiggestContour==i){
                            minRect[i]=minAreaRect(contours[i]);
                            for(size_t k=0;k<hull[i].size();k++){
                                int ind=hull[i][k];
                                hullPoint[i].push_back(contours[i][ind]);
                            }
                            count =0;

                            for(size_t k=0;k<defects[i].size();k++){
                                if(defects[i][k][3]>13*256){
                                 /*   int p_start=defects[i][k][0];   */
                                    int p_end=defects[i][k][1];
                                    int p_far=defects[i][k][2];
                                    defectPoint[i].push_back(contours[i][p_far]);
                                    circle(img_roi,contours[i][p_end],3,Scalar(0,255,0),2);
                                    // Contador do convexo hull da primeira divisão
                                    count++;
                                }

                            }

                            drawContours(img_threshold, contours, i,Scalar(255,255,0),2, 8, vector<Vec4i>(), 0, Point() );
                            drawContours(img_threshold, hullPoint, i, Scalar(255,255,0),1, 8, vector<Vec4i>(),0, Point());
                            drawContours(img_roi, hullPoint, i, Scalar(0,0,255),2, 8, vector<Vec4i>(),0, Point() );
                            approxPolyDP(contours[i],contours_poly[i],3,false);
                            boundRect[i]=boundingRect(contours_poly[i]);
                            rectangle(img_roi,boundRect[i].tl(),boundRect[i].br(),Scalar(255,0,0),2,8,0);
                            minRect[i].points(rect_point);

                            for(size_t k=0;k<4;k++){
                                line(img_roi,rect_point[k],rect_point[(k+1)%4],Scalar(0,255,0),2,8);
                            }
                        }
                    }

                }
        }

        // Caso detecte algum contorno na imagem
        if(contours2.size()>0){
                size_t indexOfBiggestContour = -1;
                size_t sizeOfBiggestContour = 0;

                for (size_t i = 0; i < contours2.size(); i++){
                    if(contours2[i].size() > sizeOfBiggestContour){
                        sizeOfBiggestContour = contours2[i].size();
                        indexOfBiggestContour = i;
                    }
                }

                vector<vector<int> >hull(contours2.size());
                vector<vector<Point> >hullPoint(contours2.size());
                vector<vector<Vec4i> >defects(contours2.size());
                vector<vector<Point> >defectPoint(contours2.size());
                vector<vector<Point> >contours_poly(contours2.size());
                Point2f rect_point[4];
                vector<RotatedRect>minRect(contours2.size());
                vector<Rect> boundRect(contours2.size());

                // Utilizando o convexo hull para contar a quantidade de dedos
                for(size_t i=0;i<contours2.size();i++){
                    if(contourArea(contours2[i])>5000){
                        convexHull(contours2[i],hull[i],true);
                        convexityDefects(contours2[i],hull[i],defects[i]);
                        if(indexOfBiggestContour==i){
                            minRect[i]=minAreaRect(contours2[i]);
                            for(size_t k=0;k<hull[i].size();k++){
                                int ind=hull[i][k];
                                hullPoint[i].push_back(contours2[i][ind]);
                            }
                            count2 =0;

                            for(size_t k=0;k<defects[i].size();k++){
                                if(defects[i][k][3]>13*256){
                                 /*   int p_start=defects[i][k][0];   */
                                    int p_end=defects[i][k][1];
                                    int p_far=defects[i][k][2];
                                    defectPoint[i].push_back(contours2[i][p_far]);
                                    circle(img_roi2,contours2[i][p_end],3,Scalar(0,255,0),2);
                                    // Contador do convexo hull da segunda divisão
                                    count2++;
                                }

                            }

                            // Limpar texto
                            strcpy(a,"");
                            strcpy(c,"");

                            // Somando a quantidade de dedos detectada nas duas imagens
                            countResultado = count + count2;

                            if(countResultado==1)
                                strcpy(a,"1");
                            else if(countResultado==2)
                                strcpy(a,"2");
                            else if(countResultado==3)
                                strcpy(a,"3");
                            else if(countResultado==4)
                                strcpy(a,"4");
                            else if(countResultado==5)
                                strcpy(a,"5");
                            else if(countResultado==6)
                                strcpy(a,"6");
                            else if(countResultado==7)
                                strcpy(a,"7");
                            else if(countResultado==8)
                                strcpy(a,"8");
                            else if(countResultado==9)
                                strcpy(a,"9");
                            else if(countResultado==10)
                                strcpy(a,"10");
                            else
                                strcpy(c,"Coloque a mao na posicao correta !!");

                            // Colocar números no frame, através da variável "a"
                            // (imagem, texto, ponto_inicial, fonte, tamanho, cor, espessura, tipo_linha, false)
                            putText(img,a,Point(70,70),CV_FONT_HERSHEY_SIMPLEX,3,Scalar(255,0,0),2,8,false);

                            // Colocar texto no frame, através da variável "b"
                            // (imagem, texto, ponto_inicial, fonte, tamanho, cor, espessura, tipo_linha, false)
                            putText(img,c,Point(10,50),CV_FONT_NORMAL,0.9,Scalar(0,0,255),2,8,false);

                            // Desenhando na imagem para mostrar a detecção dos dedos
                            drawContours(img_threshold2, contours2, i,Scalar(255,255,0),2, 8, vector<Vec4i>(), 0, Point() );
                            drawContours(img_threshold2, hullPoint, i, Scalar(255,255,0),1, 8, vector<Vec4i>(),0, Point());
                            drawContours(img_roi2, hullPoint, i, Scalar(0,0,255),2, 8, vector<Vec4i>(),0, Point() );
                            approxPolyDP(contours2[i],contours_poly[i],3,false);
                            boundRect[i]=boundingRect(contours_poly[i]);
                            rectangle(img_roi2,boundRect[i].tl(),boundRect[i].br(),Scalar(255,0,0),2,8,0);
                            minRect[i].points(rect_point);

                            for(size_t k=0; k<4; k++){
                                line(img_roi2,rect_point[k],rect_point[(k+1)%4],Scalar(0,255,0),2,8);
                            }

                        }
                    }
             }

            // Mostrar as imagens
            imshow("Original_image", img);
            imshow("Gray_image", img_gray);
            imshow("Thresholded_image", img_threshold);
            imshow("Thresholded_image 2", img_threshold2);
            imshow("ROI", img_roi);
            imshow("ROI 2", img_roi2);

            // Aguardando 30 microsegundos para dá tempo do processamento da imagem
            if(waitKey(30) == 27){
                return -1;
            }

        }
    }
     return 0;
}
