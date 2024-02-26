#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <math.h>
#include <tchar.h>
#include <windows.h>
#include <fstream>
#include <stdlib.h>
#include <iostream>

using namespace std;

#define FILE_MENU_CLEAR 1
#define FILE_MENU_SAVE 2
#define FILE_MENU_LOAD 3
#define FILE_MENU_EXIT 4
#define CHOOSE_COLOR 5

#define CLIPPING_LINE 6
#define CLIPPING_POINT 7

#define FILLING 8

#define CIRCLE_MENU_DIRECT 11
#define CIRCLE_MENU_POLAR 12
#define CIRCLE_MENU_ITERATIVE 13
#define CIRCLE_MENU_MIDPOINT 14
#define CIRCLE_MENU_MODMID 15

#define LINE_MENU_DDA 16
#define LINE_MENU_PARAMETRIC 17
#define LINE_MENU_MIDPOINT 18

#define ELLIPSE_MENU_MID 19
#define ELLIPSE_MENU_POLAR 20

LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

void addMenus(HWND);
COLORREF ChangeColor();
void load(HWND, HDC &);
void save(HWND &);

HMENU hmenu;
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");
COLORREF color;

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
    HWND hwnd;
    MSG messages;
    WNDCLASSEX wincl;

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);

    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_HAND);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;

    wincl.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));

    if (!RegisterClassEx (&wincl))
        return 0;

    hwnd = CreateWindowEx (0, szClassName, _T("2D-Project"), WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, HWND_DESKTOP, NULL, hThisInstance, NULL);

    ShowWindow (hwnd, nCmdShow);

    while (GetMessage (&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return messages.wParam;
}

/* ALL THE CIRCLE ALGORITHMS */
int rr, x_c, y_c, x_1, y_1;
void Draw_Eight(HDC hdc, int x, int y, int xc, int yc, COLORREF color)
{
    SetPixel(hdc, xc+x,yc+y, color);
    SetPixel(hdc, xc-x,yc+y, color);
    SetPixel(hdc, xc-x,yc-y, color);
    SetPixel(hdc, xc+x,yc-y, color);
    SetPixel(hdc, xc+y,yc+x,color);
    SetPixel(hdc, xc-y,yc+x,color);
    SetPixel(hdc, xc+y,yc-x,color);
    SetPixel(hdc, xc-y,yc-x,color);
}
void Mid_Point(HDC hdc, int xc, int yc, int r, COLORREF color)
{
    int x=0, y=r;
    double d =1-r;
    while(x<y)
    {
        if(d<=0)
        {
            d=d+(2*x+3);
            x++;
        }
        else
        {
            d=d+(2*(x-y)+5);
            x++;
            y--;
        }
        Draw_Eight(hdc, x, y, xc, yc, color);
    }
}
void Polar_Circle(HDC hdc,  int xc, int yc, int r, COLORREF color)
{
    int x = r, y = 0;
    double theta = 0, D_theta = 1.0/r;
    Draw_Eight(hdc, x, y, xc, yc, color);
    while(x > y)
    {
        theta += D_theta;
        x = r * cos(theta);
        y = r * sin(theta);
        Draw_Eight(hdc, x, y, xc, yc, color);
    }
}
void Iterative_Polar(HDC hdc,  int xc, int yc, int r, COLORREF color)
{
    double x = r, y = 0, D_theta = 1.0/r, C = cos(D_theta), S = sin(D_theta);
    Draw_Eight(hdc, x, y, xc, yc, color);
    while( x > y)
    {
        double X = (x * C) - (y * S);
        y = (x * S)  + (y * C);
        x = X;
        Draw_Eight(hdc, x, y, xc, yc, color);
    }
}
void Modified_Mid_Point(HDC hdc,  int xc, int yc, int r, COLORREF color)
{
    int x = 0, y = r;
    Draw_Eight(hdc, x, y, xc, yc, color);
    int d = 1-r, d1 = 3, d2 = 5 - (2*r);
    while(x < y)
    {
        if(d < 0)
        {
            x++;
            d += d1;
            d1 += 2;
            d2 += 2;
        }
        else
        {
            x++;
            y--;
            d += d2;
            d1 += 2;
            d2 += 4;
        }
        Draw_Eight(hdc, x, y, xc, yc, color);
    }
}
void Direct_Circle(HDC hdc,  int xc, int yc, int r, COLORREF color)
{
    int x = 0, y = r, R2 = r*r;
    Draw_Eight(hdc, x, y, xc, yc, color);
    while(x < y)
    {
        x++;
        y = round(sqrt( (double)(R2 - (x*x) ) ) );
        Draw_Eight(hdc, x, y, xc, yc, color);
    }
}
/* END THE CIRCLE ALGORITHMS */

/* ALL THE LINE ALGORITHMS */
int a1,a2,b1,b2;
int abs(int num)
{
    if (num > 0)
        return num;
    else
        return (num * (-1));
}
int Round(double x)
{
    return (int)(x+0.5);
}

void DDA_Line(HDC hdc,int x0, int y0, int x1, int y1, COLORREF color)
{

    int dy = y1 - y0 ;
    int dx = x1 - x0 ;

    int x, xinc ;
    double y, yinc ;

    SetPixel(hdc,x0,y0,color);
    if(abs(dx)>=abs(dy))
    {
        x=x0;
        xinc = dx>0?1:-1;
        y=y0;
        yinc=(double)dy/dx*xinc;
        while(x!=x1)
        {
            x+=xinc;
            y+=yinc;
            SetPixel(hdc,x,Round(y),color);
        }
    }
    else
    {
        int y=y0,yinc= dy>0?1:-1;
        double x=x0,xinc=(double)dx/dy*yinc;
        while(y!=y1)
        {
            x+=xinc;
            y+=yinc;
            SetPixel(hdc,Round(x),y,color);
        }
    }
}

void Parametric_Line(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color)
{
    double dx = x2-x1;
    double dy = y2-y1;
    for(double t=0; t < 1 ; t+=0.001)
    {
        int x = x1+(dx*t);
        int y = y1+(dy*t);
        SetPixel(hdc, x, y, color);
    }
}

void Mid_Point_Line(HDC hdc,int x0, int y0, int x1, int y1, COLORREF color)
{
    int dy = y1 - y0 ;
    int dx = x1 - x0 ;
    double slope = (((double)dy)/dx);

    if(slope>0&&slope<1)
    {
        if(dx < 0){
            swap(x0,x1);
            swap(y0,y1);
            dx = -dx;
            dy = -dy;
        }
     int x, y, p, change1, change2 ;
     x=x0;
     y=y0;
     p = dx - 2*dy;
     change1 = 2*(dx-dy);
     change2 = -2*dy;
     SetPixel(hdc,x,y,color);
     while(x<x1){
        if(p<=0){
            p+=change1;
            y++;
        }
        else{
            p+=change2;
        }
    x++;
    SetPixel(hdc,x,y,color);
     }
    }
    else if(slope>-1&&slope<0){
        if(dx > 0){
            swap(x0,x1);
            swap(y0,y1);
            dx = -dx;
            dy = -dy;
        }
     int x, y, p, change1, change2 ;
     x=x0;
     y=y0;
     p = dx + 2*dy;
     change1 = 2*(dx+dy);
     change2 = 2*dy;
     SetPixel(hdc,x,y,color);
     while(x>x1){
        if(p>=0){
            p+=change1;
            y++;
        }
        else{
            p+=change2;
        }
    x--;
    SetPixel(hdc,x,y,color);
     }
    }
    else if(slope>1)
    {
        if(dy < 0){
            swap(x0,x1);
            swap(y0,y1);
            dx = -dx;
            dy = -dy;
        }
     int x, y, p, change1, change2 ;
     x=x0;
     y=y0;
     p = 2*dx - dy;
     change1 = 2*(dx-dy);
     change2 = 2*dx;
     SetPixel(hdc,x,y,color);
     while(y<y1){
        if(p>=0){
            p+=change1;
            x++;
        }
        else{
            p+=change2;
        }
    y++;
    SetPixel(hdc,x,y,color);
     }
    }
    else {
        if(dy < 0){
            swap(x0,x1);
            swap(y0,y1);
            dx = -dx;
            dy = -dy;
        }
     int x, y, p, change1, change2 ;
     x=x0;
     y=y0;
     p = 2*dx + dy;
     change1 = 2*(dx+dy);
     change2 = 2*dx;
     SetPixel(hdc,x,y,color);
     while(y<y1){
        if(p<=0){
            p+=change1;
            x--;
        }
        else{
            p+=change2;
        }
    y++;
    SetPixel(hdc,x,y,color);
     }
    }
}
/* END THE LINE ALGORITHMS */

/* ALL THE ELLIPSE ALGORITHMS */
int  rx, ry, x_2, y_2, counter=0;
void draw_polar_ellipse(HDC hdc, int x, int y, int xc, int yc, COLORREF color)
{
    SetPixel(hdc, xc+x,yc+y, color);
    SetPixel(hdc, xc-x,yc+y, color);
    SetPixel(hdc, xc-x,yc-y, color);
    SetPixel(hdc, xc+x,yc-y,color);
}
void draw_midpoint_ellipse(HDC hdc, int x, int y, int xc, int yc, COLORREF color)
{
    SetPixel(hdc, xc+x,yc+y, color);
    SetPixel(hdc, xc-x,yc+y, color);
    SetPixel(hdc, xc-x,yc-y, color);
    SetPixel(hdc, xc+x,yc-y, color);
}
void polar_ellipse(HDC hdc, int xc,int yc, int B,int A,COLORREF color)
{

    int x=0, y=A;
    double theta=0;
    double dtheta = 1.0/max(B,A);
    int theta_end=90;
    while(theta<=theta_end)
    {
        draw_polar_ellipse(hdc,x,y,xc,yc,color);
        x=(A*cos(theta));
        y=(B*sin(theta));
        theta+= dtheta;
    }
}
void midpoint_ellipse(HDC hdc, int xc,int yc, int rx,int ry,COLORREF color)
{
    int x=0,y=ry,x2,y2;
    float p1=(ry*ry)-(rx*rx*ry)+(rx*rx)/4;
    int a=2*ry*ry*x;
    int b=2*rx*rx*y;
    while(a<=b)
    {
        draw_midpoint_ellipse(hdc,x,y,xc,yc,color);
        x++;
        if(p1<0)
        {
            a=2*ry*ry*x;
            p1=p1+a+(ry*ry);
        }
        else
        {
            y--;
            a=2*ry*ry*x;
            b=2*rx*rx*y;
            p1=p1+a-b+(ry*ry);
        }
        draw_midpoint_ellipse(hdc,x,y,xc,yc,color);
    }
    float p2=((ry*ry)*(x+0.5)*(x+0.5))+((rx*rx)*(y-1)*(y-1))-((rx*rx)*(ry*ry));
    a=0;
    b=0;
    while(y>=0)
    {
        draw_midpoint_ellipse(hdc,x,y,xc,yc,color);
        y--;
        if(p2<0)
        {
            x++;
            a=2*ry*ry*x;
            b=2*rx*rx*y;
            p2=p2+a-b+(rx*rx);
        }
        else
        {
            b=2*rx*rx*y;
            p2=p2-b+(rx*rx);
        }
        draw_midpoint_ellipse(hdc,x,y,xc,yc,color);
    }
}
/* END THE ELLIPSE ALGORITHMS */

/* CLIPPING CODE */
union OutCode
{
    unsigned All:4;
    struct
    {
        unsigned left:1,top:1,right:1,bottom:1;
    };
};
OutCode GetOutCode(double x,double y,int xleft,int ytop,int xright,int ybottom)
{
    OutCode out;
    out.All=0;
    if(x<xleft)
        out.left=1;
    else if(x>xright)
        out.right=1;
    if(y<ytop)
        out.top=1;
    else if(y>ybottom)
        out.bottom=1;
    return out;
}
void VIntersect(double xs,double ys,double xe,double ye,int x,double *xi,double *yi)
{
    *xi=x;
    *yi=ys+(x-xs)*(ye-ys)/(xe-xs);
}
void HIntersect(double xs,double ys,double xe,double ye,int y,double *xi,double *yi)
{
    *yi=y;
    *xi=xs+(y-ys)*(xe-xs)/(ye-ys);
}
void CohenSuth(HDC hdc,int xs,int ys,int xe,int ye,int xleft,int ytop,int xright,int ybottom)
{
    double x1=xs,y1=ys,x2=xe,y2=ye;
    OutCode out1=GetOutCode(x1,y1,xleft,ytop,xright,ybottom);
    OutCode out2=GetOutCode(x2,y2,xleft,ytop,xright,ybottom);
    while( (out1.All || out2.All) && !(out1.All & out2.All))
    {
        double xi,yi;
        if(out1.All)
        {
            if(out1.left)
                VIntersect(x1,y1,x2,y2,xleft,&xi,&yi);
            else if(out1.top)
                HIntersect(x1,y1,x2,y2,ytop,&xi,&yi);
            else if(out1.right)
                VIntersect(x1,y1,x2,y2,xright,&xi,&yi);
            else
                HIntersect(x1,y1,x2,y2,ybottom,&xi,&yi);
            x1=xi;
            y1=yi;
            out1=GetOutCode(x1,y1,xleft,ytop,xright,ybottom);
        }
        else
        {
            if(out2.left)
                VIntersect(x1,y1,x2,y2,xleft,&xi,&yi);
            else if(out2.top)
                HIntersect(x1,y1,x2,y2,ytop,&xi,&yi);
            else if(out2.right)
                VIntersect(x1,y1,x2,y2,xright,&xi,&yi);
            else
                HIntersect(x1,y1,x2,y2,ybottom,&xi,&yi);
            x2=xi;
            y2=yi;
            out2=GetOutCode(x2,y2,xleft,ytop,xright,ybottom);
        }
    }
    if(!out1.All && !out2.All)
    {

        Parametric_Line(hdc,x1,y1,x2,y2,color);
    }
}

int X_start,X_end,Y_start,Y_end,X_left,Y_top,X_right,Y_bottom;
/* END OF LINE CLIPPING ALGO */

void PointClipping(HDC hdc, int x,int y,int xleft,int ytop,int xright,int ybottom,COLORREF color)
{
    if(xleft<xright&&ytop<ybottom){
            if(x>=xleft && x<= xright && y>=ytop && y<=ybottom)
                        SetPixel(hdc,x,y,color);
    }

    else if(xleft>xright&&ytop<ybottom){
            if(x<=xleft && x>= xright && y>=ytop && y<=ybottom)
                        SetPixel(hdc,x,y,color);
    }

    else if(xleft<xright&&ytop>ybottom){
    if(x>=xleft && x<= xright && y<=ytop && y>=ybottom)
         SetPixel(hdc,x,y,color);
    }

    else if(xleft>xright&&ytop>ybottom){
        if(x<=xleft && x>= xright && y<=ytop && y>=ybottom)
                        SetPixel(hdc,x,y,color);

    }
}

int X1,Y1,X_left1,Y_top1,X_right1,Y_bottom1;
/* END OF Point CLIPPING ALGO */

/* FILLING CODE */
int xF = -1,yF = -1,rrF = -1;
bool isInside(int circleX,int circleY,int rad,int x,int y)
{
    if ((x - circleX) * (x - circleX) + (y - circleY) * (y - circleY) <= rad * rad)
        return true;
    else
        return false;
}
void drawLines(HDC hdc,int x,int y,int x_c,int y_c,int qNum)
{
    if(qNum==1)
    {
        Parametric_Line(hdc,x_c+x,y_c-y,x_c,y_c,color);
        Parametric_Line(hdc,x_c+y,y_c-x,x_c,y_c,color);
    }
    else if(qNum==2)
    {
        Parametric_Line(hdc,x_c-x,y_c+y,x_c,y_c,color);
        Parametric_Line(hdc,x_c-y,y_c+x,x_c,y_c,color);
    }
    else if(qNum==3)
    {
        Parametric_Line(hdc,x_c-x,y_c-y,x_c,y_c,color);
        Parametric_Line(hdc,x_c-y,y_c-x,x_c,y_c,color);
    }
    else if(qNum==4)
    {
        Parametric_Line(hdc,x_c+x,y_c+y,x_c,y_c,color);
        Parametric_Line(hdc,x_c+y,y_c+x,x_c,y_c,color);
    }
}
void filling(HDC hdc,int x_c,int y_c, int rr,int qNum)
{

    int x=0, y=rr, d=1-rr, c1=3, c2=5-(2*rr);
    drawLines(hdc,x,y,x_c,y_c,qNum);
    while(x<y)
    {
        if(d<0)
        {
            d+=c1;
            c2+=2;
        }
        else
        {
            d+=c2;
            c2+=4;
            y--;
        }
        c1+=2;
        x++;
        drawLines(hdc,x,y,x_c,y_c,qNum);
    }
}
void quarterNum(HDC hdc,int x,int y)
{
    int qNum;
    if(isInside(x,y,rrF,xF,yF))
    {
        if(x > xF && y < yF)
        {
            qNum = 1;
        }
        else if(x < xF && y > yF)
        {
            qNum = 2;
        }
        else if(x < xF && y < yF)
        {
            qNum = 3;
        }
        else if(x > xF && y > yF)
        {
            qNum = 4;
        }
    }
    filling(hdc,xF,yF,rrF,qNum);
}
/* END OF FILLING ALGO*/

int choice = 0;
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc = GetDC(hwnd);
    switch (message)
    {
    case WM_LBUTTONDBLCLK:
    {
        switch(choice)
        {
        /* Line clipping */
        case 1:
            X_left=LOWORD(lParam);
            Y_top=HIWORD(lParam);
            choice++;
            break;
        case 2:
            X_right=LOWORD(lParam);
            Y_bottom=HIWORD(lParam);
            Rectangle(hdc, X_left,Y_top,X_right,Y_bottom);
            choice++;
            break;
        case 3:
            X_start=LOWORD(lParam);
            Y_start=HIWORD(lParam);
            choice++;
            break;
        case 4:
            X_end=LOWORD(lParam);
            Y_end=HIWORD(lParam);
            CohenSuth(hdc,X_start,Y_start,X_end,Y_end,X_left,Y_top,X_right,Y_bottom);
            choice=3;
            break;

        /* point clipping */
        case 5:
            X_left1=LOWORD(lParam);
            Y_top1=HIWORD(lParam);
            choice++;
            break;
        case 6:
            X_right1=LOWORD(lParam);
            Y_bottom1=HIWORD(lParam);
            Rectangle(hdc, X_left1,Y_top1,X_right1,Y_bottom1);
            choice++;
            break;
        case 7:
            X1=LOWORD(lParam);
            Y1=HIWORD(lParam);
            choice++;
            PointClipping(hdc,X1,Y1,X_left1,Y_top1,X_right1,Y_bottom1, 1000);
            choice=7;
            break;

        /*filling*/
        case 8:
            x_c=LOWORD(lParam);
            y_c = HIWORD(lParam);
            quarterNum(hdc,x_c,y_c);
            break;

        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            x_c=LOWORD(lParam);
            y_c = HIWORD(lParam);
            break;

        case 16:
        case 17:
        case 18:
            a1= LOWORD(lParam);
            b1= HIWORD(lParam);
            break;

        case 19:
        case 20:
            x_c=LOWORD(lParam);
            y_c = HIWORD(lParam);
            break;
        }
        break;
    }
    case WM_RBUTTONDBLCLK:
    {
        switch(choice)
        {
        case 11:
            x_1= LOWORD (lParam);
            y_1= HIWORD(lParam);
            rr=sqrt(pow((x_1-x_c),2)+pow((y_1-y_c),2));
            xF = x_c;
            yF = y_c;
            rrF = rr;
            Mid_Point(hdc, x_c,y_c,rr, color);
            break;
        case 12:
            x_1= LOWORD (lParam);
            y_1= HIWORD(lParam);
            rr=sqrt(pow((x_1-x_c),2)+pow((y_1-y_c),2));
            xF = x_c;
            yF = y_c;
            rrF = rr;
            Polar_Circle(hdc, x_c,y_c,rr, color);
            break;
        case 13:
            x_1= LOWORD (lParam);
            y_1= HIWORD(lParam);
            rr=sqrt(pow((x_1-x_c),2)+pow((y_1-y_c),2));
            xF = x_c;
            yF = y_c;
            rrF = rr;
            Iterative_Polar(hdc, x_c,y_c,rr, color);
            break;
        case 14:
            x_1= LOWORD (lParam);
            y_1= HIWORD(lParam);
            rr=sqrt(pow((x_1-x_c),2)+pow((y_1-y_c),2));
            xF = x_c;
            yF = y_c;
            rrF = rr;
            Modified_Mid_Point(hdc, x_c,y_c,rr, color);
            break;
        case 15:
            x_1= LOWORD (lParam);
            y_1= HIWORD(lParam);
            rr=sqrt(pow((x_1-x_c),2)+pow((y_1-y_c),2));
            xF = x_c;
            yF = y_c;
            rrF = rr;
            Direct_Circle(hdc, x_c,y_c,rr, color);
            break;

        case 16:
            a2=LOWORD(lParam);
            b2= HIWORD(lParam);
            DDA_Line(hdc, a1, b1,  a2, b2, color);
            break;
        case 17:
            a2=LOWORD(lParam);
            b2= HIWORD(lParam);
            Parametric_Line(hdc, a1, b1,  a2, b2, color);
            break;
        case 18:
            a2=LOWORD(lParam);
            b2= HIWORD(lParam);
            Mid_Point_Line(hdc, a1, b1,  a2, b2, color);
            break;

        case 19:
        {
            if(counter == 0)
            {
                x_1= LOWORD (lParam);
                y_1= HIWORD(lParam);
                rx=sqrt(pow((x_1-x_c),2)+pow((y_1-y_c),2));
                counter++;
            }
            else if(counter ==1)
            {
                x_2= LOWORD (lParam);
                y_2= HIWORD(lParam);
                ry=sqrt(pow((x_2-x_c),2)+pow((y_2-y_c),2));
                SetPixel(hdc, x_c,y_c, color);
                midpoint_ellipse(hdc, x_c,y_c,rx,ry, color);
            }
            break;
        }
        case 20:
        {
            if(counter ==0 )
            {
                x_1= LOWORD (lParam);
                y_1= HIWORD(lParam);
                rx=sqrt(pow((x_1-x_c),2)+pow((y_1-y_c),2));
                counter++;
            }
            else if(counter ==1)
            {
                x_2= LOWORD (lParam);
                y_2= HIWORD(lParam);
                ry=sqrt(pow((x_2-x_c),2)+pow((y_2-y_c),2));
                SetPixel(hdc, x_c,y_c, RGB(0,0,255));
                polar_ellipse(hdc, x_c,y_c,rx,ry, color);
            }
            break;
        }
        }
        break;
    }

    case WM_COMMAND:
    {
        switch(wParam)
        {
        case FILE_MENU_CLEAR:
            cout << "WINDOW CLEARED" << endl;
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case FILE_MENU_SAVE:
            cout << "FILE SAVED!" << endl;
            save(hwnd);
            break;

        case FILE_MENU_LOAD:
            cout << "FILE LOADED!" << endl;
            load(hwnd, hdc);
            break;

        case FILE_MENU_EXIT:
            cout << endl << "       GOOD BYE!!      " <<endl;
            DestroyWindow(hwnd);
            break;

        case CHOOSE_COLOR:
            color = ChangeColor();
            break;

        case CLIPPING_LINE:
            choice = 1;            /* choices from 1 to 4 are taken */
            cout << "line clipping" <<endl;
            break;

        case CLIPPING_POINT:
            choice = 5;            /* choices from 5 to 7 are taken */
            cout << "POINT CLIPPING" <<endl;
            break;

        case FILLING:
            choice=8;
            cout << "FILLING..." <<endl;
            break;

        case CIRCLE_MENU_MIDPOINT:
            choice = 11;
            cout << "MID-POINT CIRCLE" <<endl;
            break;

        case CIRCLE_MENU_POLAR:
            choice = 12;
            cout << "POLAR CIRCLE" <<endl;
            break;

        case CIRCLE_MENU_ITERATIVE:
            choice = 13;
            cout << "ITERATIVE POLAR CIRCLE"<<endl;
            break;

        case CIRCLE_MENU_MODMID:
            choice = 14;
            cout << "MODIFIED MID-POINT CIRCLE" <<endl;
            break;

        case CIRCLE_MENU_DIRECT:
            choice = 15;
            cout << "DIRECT CIRCLE" <<endl;
            break;

        case LINE_MENU_DDA:
            choice = 16;
            cout << "DDA LINE" <<endl;
            break;

        case LINE_MENU_PARAMETRIC:
            choice = 17;
            cout << "PARAMETRIC LINE" <<endl;
            break;

        case LINE_MENU_MIDPOINT:
            choice = 18;
            cout << "MID-POINT LINE" <<endl;
            break;

        case ELLIPSE_MENU_MID:
            choice = 19;
            cout << "MID-POINT ELLIPSE" <<endl;
            break;

        case ELLIPSE_MENU_POLAR:
            choice = 20;
            cout << "POLAR ELLIPSE" <<endl;
            break;
        }
        break;
    }

    case WM_CREATE:
        addMenus(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage (0);
        break;

    default:
        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

void addMenus(HWND hwnd)
{
    hmenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    HMENU hLineMenu = CreateMenu();
    HMENU hCircleMenu = CreateMenu();
    HMENU hEllipseMenu = CreateMenu();
    HMENU hClippingMenu = CreateMenu();
    HMENU hFillingMenu = CreateMenu();

    AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");
    AppendMenu(hFileMenu, MF_STRING, FILE_MENU_CLEAR, "Clear Screen");
    AppendMenu(hFileMenu,MF_STRING, CHOOSE_COLOR,"Choose Color");
    AppendMenu(hFileMenu, MF_STRING, FILE_MENU_SAVE, "Save");
    AppendMenu(hFileMenu, MF_STRING, FILE_MENU_LOAD, "Load");
    AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hFileMenu, MF_STRING, FILE_MENU_EXIT, "Exit");

    AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hLineMenu, "Line");
    AppendMenu(hLineMenu, MF_STRING, LINE_MENU_DDA, "DDA");
    AppendMenu(hLineMenu, MF_STRING, LINE_MENU_MIDPOINT, "Mid-Point");
    AppendMenu(hLineMenu, MF_STRING, LINE_MENU_PARAMETRIC, "Parametric");

    AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hCircleMenu, "Circle");
    AppendMenu(hCircleMenu, MF_STRING, CIRCLE_MENU_DIRECT, "Direct");
    AppendMenu(hCircleMenu, MF_STRING, CIRCLE_MENU_POLAR, "Polar");
    AppendMenu(hCircleMenu, MF_STRING, CIRCLE_MENU_ITERATIVE, "Iterative Polar");
    AppendMenu(hCircleMenu, MF_STRING, CIRCLE_MENU_MIDPOINT, "Mid-Point");
    AppendMenu(hCircleMenu, MF_STRING, CIRCLE_MENU_MODMID, "Modified Mid-Point");

    AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hEllipseMenu, "Ellipse");
    AppendMenu(hEllipseMenu, MF_STRING, ELLIPSE_MENU_MID, "Mid-Point");
    AppendMenu(hEllipseMenu, MF_STRING, ELLIPSE_MENU_POLAR, "Polar");

    AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hClippingMenu, "Clipping");
    AppendMenu(hClippingMenu, MF_STRING, CLIPPING_LINE, "Line");
    AppendMenu(hClippingMenu, MF_STRING, CLIPPING_POINT, "point");

    AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hFillingMenu, "Filling");
    AppendMenu(hFillingMenu, MF_STRING, FILLING, "Choose Quarter");

    SetMenu(hwnd, hmenu);
}

COLORREF ChangeColor()
{
    int R, G, B;
    cout << "ENTER THE COLOR IN FORM RGB RESPECTIVELY" <<endl;
    cin >> R >> G >> B;
    COLORREF clr = RGB(R,G,B);
    return clr;
}

bool HDCToFile(const char* FilePath, HDC Context, RECT Area, uint16_t BitsPerPixel = 24)
{
    uint32_t Width = Area.right - Area.left;
    uint32_t Height = Area.bottom - Area.top;
    BITMAPINFO Info;
    BITMAPFILEHEADER Header;
    memset(&Info, 0, sizeof(Info));
    memset(&Header, 0, sizeof(Header));
    Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    Info.bmiHeader.biWidth = Width;
    Info.bmiHeader.biHeight = Height;
    Info.bmiHeader.biPlanes = 1;
    Info.bmiHeader.biBitCount = BitsPerPixel;
    Info.bmiHeader.biCompression = BI_RGB;
    Info.bmiHeader.biSizeImage = Width * Height * (BitsPerPixel > 24 ? 4 : 3);
    Header.bfType = 0x4D42;
    Header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    char* Pixels = NULL;
    HDC MemDC = CreateCompatibleDC(Context);
    HBITMAP Section = CreateDIBSection(Context, &Info, DIB_RGB_COLORS, (void**)&Pixels, 0, 0);
    DeleteObject(SelectObject(MemDC, Section));
    BitBlt(MemDC, 0, 0, Width, Height, Context, Area.left, Area.top, SRCCOPY);
    DeleteDC(MemDC);
    std::fstream hFile(FilePath, std::ios::out | std::ios::binary);
    if (hFile.is_open())
    {
        hFile.write((char*)&Header, sizeof(Header));
        hFile.write((char*)&Info.bmiHeader, sizeof(Info.bmiHeader));
        hFile.write(Pixels, (((BitsPerPixel * Width + 31) & ~31) / 8) * Height);
        hFile.close();
        DeleteObject(Section);
        return true;
    }
    DeleteObject(Section);
    return false;
}
void load(HWND hWnd, HDC &hdc)
{
    string fileName = "picture.bmp";
    if (fileName == "")
        return ;
    HBITMAP hBitmap;
    hBitmap = (HBITMAP)::LoadImage(NULL, fileName.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    HDC hLocalDC;
    hLocalDC = CreateCompatibleDC(hdc);
    BITMAP qBitmap;
    int iReturn = GetObject(reinterpret_cast<HGDIOBJ>(hBitmap), sizeof(BITMAP),reinterpret_cast<LPVOID>(&qBitmap));
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hLocalDC, hBitmap);
    BOOL qRetBlit = BitBlt(hdc, 0, 0, qBitmap.bmWidth, qBitmap.bmHeight,hLocalDC, 0, 0, SRCCOPY);
    SelectObject (hLocalDC, hOldBmp);
    DeleteDC(hLocalDC);
    DeleteObject(hBitmap);
}
void save(HWND &hWnd)
{
    HDC hdc = GetDC(hWnd);
    string fileName = "picture.bmp";
    if (fileName == "")
        return ;
    int windowWidth ;
    int windowHeight;
    RECT rect;
    if(GetWindowRect(hWnd, &rect))
    {
        windowWidth = rect.right - rect.left;
        windowHeight = rect.bottom - rect.top;
    }
    RECT rect1 = {0, 0, windowWidth, windowHeight};
    HDCToFile(fileName.c_str(),hdc,rect1);
}
