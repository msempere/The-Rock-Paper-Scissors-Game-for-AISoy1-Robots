/*
 * AISoy Robotics 2011
 * Rock-Paper-Scissors Game for AISoy1
 * Miguel Sempere (msempere@aisoy.com)
 * v. 0.0.1.1
*/

#include <iostream>
#include <aisoy1/bot.h>
#include <aisoy1/log.h>
#include <stdio.h>
#include <string.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <math.h>
#include <sstream>
#include <time.h>
#include <vector>
#include <iterator>
#include <sstream>
#include <ctime> 
using namespace std;



AISoy1::Bot bot;
AISoy1::Image imgBot,imgFilt;

const char paper[70]="  111       1 11      1 1 1     1 1111    1    1    1    1    111111 ";
const char rock[70]="   111     11 1 1   11   1 1  1 11  11   11  1 1   1 1  1     1111   ";
const char scissors[70]="111   111 1  1 1   1 1  1    1  1  1111  1  1    11  1 1   1111   111";






//obtenemos la distancia en pixeles entre dos puntos. 
//La magnitud de la recta
float  Distance( CvPoint pt1, CvPoint pt2 ) 
{ 
    int dx = pt2.x - pt1.x; 
    int dy = pt2.y - pt1.y; 
 
    return cvSqrt( (float)(dx*dx + dy*dy)); 
} 


//obtenemos el punto mitad de una recta definida por dos puntos
CvPoint mitadSegmento(CvPoint p1,CvPoint p2){
	
	int x=(p1.x+p2.x)/2;
	int y=(p1.y+p2.y)/2;
	
	return cvPoint(x,y);
}


struct Vector
{
    double x, y;
};


//obtenemos el angulo entre dos vectores
double getRotateAngle(Vector vec1, Vector vec2)
{
	const double epsilon = 1.0e-6;
	const double PI = acos(-1.0); // 180 degree
	double angle = 0;


	Vector norVec1, norVec2;
	norVec1.x = vec1.x / sqrt(pow(vec1.x, 2) + pow(vec1.y, 2));
	norVec1.y = vec1.y / sqrt(pow(vec1.x, 2) + pow(vec1.y, 2));
	norVec2.x = vec2.x / sqrt(pow(vec2.x, 2) + pow(vec2.y, 2));
	norVec2.y = vec2.y / sqrt(pow(vec2.x, 2) + pow(vec2.y, 2));


	double dotProd = (norVec1.x * norVec2.x) + (norVec1.y * norVec2.y);
	if ( abs(dotProd - 1.0) <= epsilon )
		angle = 0;
	else if ( abs(dotProd + 1.0) <= epsilon )
		angle = PI;
	else {
		double cross = 0;
		angle = acos(dotProd);
		
		cross = (norVec1.x * norVec2.y) - (norVec2.x * norVec1.y);

		if (cross < 0) 
				angle = 2 * PI - angle;
		}

		return angle*(180 / PI);
	
}

 
std::string convert(int number)
{
   std::ostringstream sin;
   sin << number;
   std::string val = sin.str();
   return val;
}





int fingers(){	
	
	imgBot=bot.captureImage();
	//imgBot.save("html/imageBotOriginal.jpg");
	//imgBot.save("html/imageBotFiltered.jpg");


	
	int c = 0;
	CvSeq* a = 0;
	int cont=0;
	IplImage *imgOriginal=NULL;

	imgOriginal=imgBot.toIplImage();
	CvSize sz=cvGetSize(imgOriginal);
	IplImage* src = cvCreateImage( sz, 8, 3 );
	IplImage* hsv_image = cvCreateImage( sz, 8, 3);
	IplImage* hsv_mask = cvCreateImage( sz, 8, 1);
	IplImage* hsv_edge = cvCreateImage( sz, 8, 1);
	CvScalar  hsv_min = cvScalar(0, 37, 92, 0);
	CvScalar  hsv_max = cvScalar(255, 94, 180, 0);
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvMemStorage* areastorage = cvCreateMemStorage(0);
	CvMemStorage* minStorage = cvCreateMemStorage(0);
	CvMemStorage* dftStorage = cvCreateMemStorage(0);
	CvSeq* contours = NULL;


	try{
		

			IplImage* bg = cvCreateImage( sz, 8, 3);
			//cvRectangle( bg, cvPoint(0,0), cvPoint(bg->width,bg->height), CV_RGB( 255, 255, 255), -1, 8, 0 );
			bg->origin = 1;

			/* si no vamos a mostrar imagenes no seria necesario mostrar las lineas
			for(int b = 0; b< int(bg->width/10); b++)
			{
				cvLine( bg, cvPoint(b*20, 0), cvPoint(b*20, bg->height), CV_RGB( 200, 200, 200), 1, 8, 0 );
				cvLine( bg, cvPoint(0, b*20), cvPoint(bg->width, b*20), CV_RGB( 200, 200, 200), 1, 8, 0 );
			}
			*/
	

			//conversion de RGB a HSV
			src=imgOriginal;
			cvCvtColor(src, hsv_image, CV_BGR2HSV);

		
			//realizamos el filtrado
			cvInRangeS (hsv_image, hsv_min, hsv_max, hsv_mask);
			//AISoy1::Image tempImg1(hsv_mask);
			//tempImg1.save("html/hsv_image.jpg");
			
			
    
                     
			//realizamos un Smooth
			cvSmooth( hsv_mask, hsv_mask, CV_MEDIAN, 13, 0, 0, 0 );//default 27
			//AISoy1::Image tempImg2(hsv_mask);
			//tempImg2.save("html/hsv_mask.jpg");
			
			
			//realizamos un closing	
			//creamos un elemento estructurante rectangular para ralizar un closing de la imagen
			IplConvKernel* kernel = NULL;
			kernel = cvCreateStructuringElementEx( 5, 5, 1, 1, CV_SHAPE_RECT, 0 );
			cvMorphologyEx( hsv_mask, hsv_mask, NULL, kernel, CV_MOP_CLOSE, 3 );
			cvReleaseStructuringElement(&kernel);
			
			//realizamos Canny
			cvCanny(hsv_mask, hsv_edge, 1, 3, 5);
			//AISoy1::Image tempImg3(hsv_mask);
			//tempImg3.save("html/hsv_mask_canny.jpg");


			

			//buscamos los contornos en la imagen. Equivalente a los blobs
			cvFindContours( hsv_mask, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
			CvSeq* contours2 = NULL;
			double result = 0, result2 = 0;

			while(contours)
			{

				result = fabs( cvContourArea( contours, CV_WHOLE_SEQ ) );
				
				//nos quedamos con el blob mas grande
				if ( result > result2) {result2 = result; contours2 = contours;};
					contours  =  contours->h_next;

			}
			
			//si existe el blob mayor, efectuamos los calculos
			if ( contours2 )
			{

				CvRect rect = cvBoundingRect( contours2, 0 );
				cvRectangle( bg, cvPoint(rect.x, rect.y + rect.height), cvPoint(rect.x + rect.width, rect.y), CV_RGB(200, 0, 200), 1, 8, 0 );

				int checkcxt = cvCheckContourConvexity( contours2 );

				
				//obtenemos la secuencia de hull CV_COUNTER_CLOCKWISE o CV_CLOCKWISE
				CvSeq* hull = cvConvexHull2( contours2, 0, CV_COUNTER_CLOCKWISE, 0 );
				//CvSeq* hull = cvConvexHull2( contours2, 0, CV_CLOCKWISE, 0 );
				
				CvSeq* defect = cvConvexityDefects( contours2, hull, dftStorage );			
				CvBox2D box = cvMinAreaRect2( contours2, minStorage );
				
				
				//almacenamos la primera posicion de los defects para futuras operaciones con ellos
				CvSeq* first_contour=defect;
				
				CvConvexityDefect* defectArray;
				vector<CvPoint> dedos;
				CvPoint* pointArray;
				Vector v1,v2;
				
				
				for(;defect;defect = defect->h_next)
				{
					int nomdef = defect->total; //cantidad de defects
					
					if(nomdef == 0)
						continue;
					 
					//reserva de memoria para los defects 
					defectArray = (CvConvexityDefect*)malloc(sizeof(CvConvexityDefect)*nomdef);
					
					
					//cogemos el set de defects
					cvCvtSeqToArray(defect,defectArray, CV_WHOLE_SEQ);
					
					// Dibujamos mar as para los defects
					for(int i=0; i<nomdef; i++)
					{
						
						float dist=sqrt(pow((defectArray[i].end->x-defectArray[i].start->x),2)+pow((defectArray[i].end->y-defectArray[i].start->y),2));
						float profundidad=Distance(mitadSegmento(*(defectArray[i].start),*(defectArray[i].end)),*(defectArray[i].depth_point));
						
						
						
						v2.x=defectArray[i].depth_point->x-defectArray[i].start->x;
						v2.y=defectArray[i].depth_point->y-defectArray[i].start->y;
						v1.x=defectArray[i].depth_point->x-defectArray[i].end->x;
						v1.y=defectArray[i].depth_point->y-defectArray[i].end->y;

						float angulo=getRotateAngle(v1,v2);
						
						
						
						
						//realizamos un filtrado de defects y nos quedamos con los que son dedos
						if(dist>=30 && profundidad>=50 && angulo<=100){
							
							//almacenamos los datos de posibles dedos
							dedos.push_back(*(defectArray[i].start));
							dedos.push_back(*(defectArray[i].end));
							
							//cout<<defectArray[i].depth<<" - distancia: "<<dist<<" profundida: "<<profundidad<<" angulo: "<<angulo<<" - SI Dibujado"<<endl;
						}
						//else
							//cout<<defectArray[i].depth<<" - distancia: "<<dist<<" profundida: "<<profundidad<<" angulo: "<<angulo<<" - NO Dibujado"<<endl;
					}
					
					
					int tam=dedos.size();
					
					//realizamos la comprobacion de los mejores defects.
					//pintamos sus marcas y comprobamos el numero final de los que corresponden con dedos.
					if(tam==2){
						
							if(Distance(dedos[0],dedos[1])>180){
								//cvCircle( bg, dedos[1], 15, CV_RGB(124, 252, 0), 2, 8, 0 );
								cont=1;
							}
						
							else{
								//cvCircle( bg, dedos[0], 15, CV_RGB(124, 252, 0), 2, 8, 0 );
								//cvCircle( bg, dedos[1], 15, CV_RGB(124, 252, 0), 2, 8, 0 );
								cont=2;
							}
					}
					
					else{
						for(int j=0; j<tam; j++) { 
						
							if(j%2==0){ //si es par
								cvCircle( bg, dedos[j], 15, CV_RGB(124, 252, 0), 2, 8, 0 );
								cont++;
							}
							else{//si no es par
								
								if(j==tam-1){ //si es el ultimo
									cvCircle( bg, dedos[j], 15, CV_RGB(124, 252, 0), 2, 8, 0 );
									cont++;
									}
							}
							
							
						}
					}

					
					 
					// liberamos memoria     
					free(defectArray);
				}
				
			}

			
			

			//dibujamos contorno
			//cvDrawContours( bg, contours2,  CV_RGB( 0, 200, 0), CV_RGB( 0, 100, 0), 1, 1, 8, cvPoint(0,0));
			
			//AISoy1::Image imgT(bg);
			//imgT.save("html/imageBotFiltered.jpg");
			
			//liberamos memoria
			
			/*
			if(&bg!=NULL)
				cvReleaseImage(&bg);
			if(src!=NULL)
				cvReleaseImage(&src);
			if(hsv_image!=NULL)
				cvReleaseImage(&hsv_image);
			if(hsv_mask!=NULL)
				cvReleaseImage(&hsv_mask);
			if(hsv_edge!=NULL)
				cvReleaseImage(&hsv_edge);
			if(imgOriginal!=NULL)
				cvReleaseImage(&imgOriginal);
			
			if(storage!=NULL)
				cvReleaseMemStorage(&storage);
			if(areastorage!=NULL)
				cvReleaseMemStorage(&areastorage);
			if(minStorage!=NULL)
				cvReleaseMemStorage(&minStorage);
			if(dftStorage!=NULL)
				cvReleaseMemStorage(&dftStorage);
			*/



		return cont;
	}

	catch( char * str ) {
      	cout << "Excepcion: " << str << '\n';
		bot.say(str);
		return -1;
   	}		
}



//generamos un numero aleatorio entre 1 y 3
int generarJugadaBot(){
	srand((unsigned)time(0)); 
	
	int valor=(rand()%3)+1;
	
	if(valor==1)
		bot.mouthDraw(rock);
	else
		if(valor==2)
			bot.mouthDraw(paper);
		else
			if(valor==3)
				bot.mouthDraw(scissors);

    return valor; 
}



//-1 gana usuario, 0 empate, 1 gana bot
int resultadoJugada(int jugadaBot){
	
	int dedos=fingers();
	
	//papel
	if(dedos>=4){
		if(jugadaBot==1)
			return  1;
		if(jugadaBot==2)
			return 0;
		if(jugadaBot==3)
			return -1;
	}
	else{
		//tijeras
		if(dedos>=1){
			if(jugadaBot==1)
				return  1;
			if(jugadaBot==2)
				return -1;
			if(jugadaBot==3)
				return 0;
		}
		//piedra
		else{
			if(jugadaBot==1)
				return  0;
			if(jugadaBot==2)
				return -1;
			if(jugadaBot==3)
				return 1;
		}
	}
	
}

std::string intToString(int n){
	
	if(n==0)
		return "cero";
	if(n==1)
		return "uno";
	if(n==2)
		return "dos";
	if(n==3)
		return "tres";
	if(n==4)
		return "cuatro";
	if(n==5)
		return "cinco";
	else
		return "no reconozco el numero";
	
}


void jugar(int numveces){
	
	int res=-2;
	int puntuacionBot=0;
	int puntuacionHumano=0;
	
	//jugaremos al mejor de X, que lo proporciona el usuario	
	while(puntuacionBot<numveces && puntuacionHumano<numveces){
		
			sleep(1);
			bot.say("Piedra, papel, tijera.");
			res=resultadoJugada(generarJugadaBot());
			
			if(res==-1){
				bot.say("Punto para ti.");
				puntuacionHumano++;
			}
			if(res==1){
				bot.say("JEJEJE. Punto para mi.");
				puntuacionBot++;
			}
				
			sleep(1);
			std:string frase="Vamos "+intToString(puntuacionHumano)+" a "+intToString(puntuacionBot)+" .";
			bot.say(frase);
	}
	
	sleep(1);
	
	if(puntuacionBot>puntuacionHumano)
		bot.say("Te he ganado. JAJAJA.");
	else
		bot.say("Quizaa deberias unirte a mi. Podriiamos ser campeones mundiales jugando por parejas.");
}



int general() {
	int mejorDe=0;
	bool salir=false;
	
	bool setGrammar=bot.setGrammarFile("./dicc.bnf");
	//bool setGrammar=bot.setGrammarFile("./apertura.bnf");
	//bool setGrammar=bot.setGrammar("si | no");
	
	bot.moveServo(AISoy1::Eyes,0.5);
	
	if(!setGrammar){
		bot.say("Ahora no estoy con animos para jugar, quizaas mas tarde.");
		return -1;
	}
	
	bot.say("Vamos a jugar a piedra, papel, tijera. Te recuerdo que soy campeon mundial de esta modalidad. Seguro que quieres desafiarme?");
	AISoy1::Sentence s=bot.listen(30);
	
	cout<<"Respuesta: "<<s.sentence()<<endl;
	
	std::string respuesta=s.sentence();
	
	if(respuesta=="si vamos a jugar" || respuesta=="claro vamos a jugar" || respuesta=="venga vamos a jugar"){
		
		while(!salir){
		
			bot.say("Jugamos?. Al mejor de cuanto?");
			s=bot.listen(30);
			
			if(s.sentence()=="uno")
				mejorDe=1;
			if(s.sentence()=="dos")
				mejorDe=2;
			if(s.sentence()=="tres")
				mejorDe=3;
			if(s.sentence()=="cuatro")
				mejorDe=4;
			if(s.sentence()=="cinco")
				mejorDe=5;
			
			
			bot.say("Estupendo, vamos cero a cero, el mejor de "+s.sentence()+" gana.");//aqui se cae
			
			jugar(mejorDe);
			sleep(1);
		}
		
	}
	else{
		if(s.sentence()=="no"){
			bot.say("Dicen que es mejor retirarse de una batalla que perder tropas sin victoria.");
			return 0;
		}
		if(s.sentence()=="infinito"){
			bot.say("Creo que tenemos mejores cosas que hacer que jugar infinitamente a un juego. Ademaas, te ganariia igual.");
			return 0;
		}
	}


	


}


int main(int argc, char **argv){
	
	cout<<"Rock-Paper-Scissors Game for AISoy1 v.0.0.1.1"<<endl;
	
	general();
	return 0;
}
