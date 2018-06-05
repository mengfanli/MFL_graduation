clear all;
%障碍和目标，起始位置都已知的路径规划,意图实现从起点可以规划出一条避开障碍到达目标的路径。
%初始化车的参数 
Xo=[0 0];
%起点位置
k=15;%计算引力需要的增益系数
K=1;%初始化
m=5;%计算斥力的增益系数，都是自己设定的。
Po=2;%障碍影响距离，当障碍和车的距离大于这个距离时，斥力为0，即不受该障碍的影响。也是自己设定。
n=50;%障碍个数
a=1;%改进的斥力函数中的引入目标距离的a次幂
l=0.2;%步长
J=200;%循环迭代次数
step=0;
start=1;
%如果不能实现预期目标，可能也与初始的增益系数，Po设置的不合适有关。
%end
%给出障碍和目标信息s
matx=rand(n,2)*10;
matx2=[10 10];
Xsum=[matx2;matx];
%Xsum(2,1)=9.8;
%Xsum(2,2)=9.9;
%load 'G:\graduation_project\simulation\Xsum.mat';
%load Xsum.mat;
%Xsum=[10 10;1 1.5;3 2.2;4 4.5;3 6;6 2;5.5 6;4.5 3;8 8.2;4 3.2;8 6;5.3 3.6;5.1 2.9;7.9 8.3;5.8 7;8 8;8.2 8.9;3 4.5;6.7 2];%这个向量是(n+1)*2维，其中[10 10]是目标位置，剩下的都是障碍的位置。
Xj=Xo;%j=1循环初始，将车的起始坐标赋给Xj
%***************初始化结束，开始主体循环******************
%for j=1:J%循环开始
j=0;
while(j<200)
    j=j+1;
Goal(j,1)=Xj(1);%Goal是保存车走过的每个点的坐标。刚开始先将起点放进该向量。
Goal(j,2)=Xj(2); %调用计算角度模块
Theta=compute_angle(Xj,Xsum,n);%Theta是计算出来的车和障碍，和目标之间的与X轴之间的夹角，统一规定角度为逆时针方向，用这个模块可以计算出来。
%调用计算引力模块
Angle=Theta(1);%Theta（1）是车和目标之间的角度，目标对车是引力。
angle_at=Theta(1);%为了后续计算斥力在引力方向的分量赋值给angle_at
[Fatx,Faty]=compute_Attract(Xj,Xsum,k,Angle,0,Po,n);%计算出目标对车的引力在x,y方向的两个分量值。
for i=1:n
angle_re(i)=Theta(i+1);%计算斥力用的角度，是个向量，因为有n个障碍，就有n个角度。
end
%调用计算斥力模块
[Frerxx,Freryy,Fataxx,Fatayy]=compute_repulsion(Xj,Xsum,m,angle_at,angle_re,n,Po,a);%计算出斥力在x,y方向的分量数组。
%计算合力和方向，这有问题，应该是数，每个j循环的时候合力的大小应该是一个唯一的数，不是数组。应该把斥力的所有分量相加，引力所有分量相加。
Fsumyj=Faty+Freryy+Fatayy;%y方向的合力
Fsumxj=Fatx+Frerxx+Fataxx;%x方向的合力
%Fsumyj=Faty+Freryy;%y方向的合力
%Fsumxj=Fatx+Frerxx;%x方向的合力
%Fsum(j)=sqrt(Fsumyj^2+Fsumxj^2);
[ Usum ]=compute_potentials (Xj,Xsum,k,m,n,Po,a);
Fsum(j)=Usum;
%Fsumyj=Faty+Freryy;%y方向的合力
%Fsumxj=Fatx+Frerxx;%x方向的合力

Position_angle=atan(Fsumyj/Fsumxj);%合力与x轴方向的夹角向量 %计算车的下一步位置
if (((Fsumyj<0)&(Fsumxj<0))|((Fsumyj>0)&(Fsumxj<0)))
Position_angle=Position_angle+pi;
end
Xnext(1)=Xj(1)+l*cos(Position_angle);
Xnext(2)=Xj(2)+l*sin(Position_angle); %保存车的每一个位置在向量中
Xj=Xnext; %判断
%if ((Xj(1)-Xsum(1,1))>0)&((Xj(2)-Xsum(1,2))>0)%是应该完全相等的时候算作到达，还是只是接近就可以？现在按完全相等的时候编程。
if (sqrt((Xj(1)-Xsum(1,1))^2+(Xj(2)-Xsum(1,2))^2)<0.15)
K=j;%记录迭代到多少次，到达目标。
break;
%记录此时的j值
end%如果不符合if的条件，重新返回循环，继续执行。

if (j>10)
    if( abs(Goal(j,1)-Goal(j-9,1))+abs(Goal(j,2)-Goal(j-9,2))<0.4)
    [ Usum ] = compute_potentials (Xj,Xsum,k,m,n,Po,a);%%%        
        start=j;
        [X_next,Fsum_next,step]=jump_minimum(Fsum(j),Xj,Xsum,k,m,n,Po,a,l);
        for k=1:step-1
            Goal(k+j,1)=X_next(k+1,1);
            Goal(k+j,2)=X_next(k+1,2);
            Fsum(k+j)=Fsum_next(k+1);
        end
        Xj(1)=X_next(step,1);
        Xj(2)=X_next(step,2);
        j=j+step-1;
    end
end

end%大循环结束 K=j;
Goal(K,1)=Xsum(1,1);%把路径向量的最后一个点赋值为目标
Goal(K,2)=Xsum(1,2);
%***********************************画出障碍，起点，目标，路径点************************* %画出路径
X=Goal(:,1);
Y=Goal(:,2);
%路径向量Goal是二维数组,X,Y分别是数组的x,y元素的集合，是两个一维数组。
%障碍的x坐标 4 3.2;5.1 2.9;7.9 8.3;5.8 7;8 8;8.2 8.9;3 4.5;6.7 2
x=Xsum(:,1);
y=Xsum(:,2);

figure(1)
plot(x,y,'o',10,10,'v',0,0,'ms',X,Y,'.-r',X(start),Y(start),'*g',X(start+step),Y(start+step),'*g');
xlabel('X/m');
ylabel('Y/m');
title('Map (X - Y)');
text(1,6,num2str(K));%相当于text(x,y,'2')
figure(2)
plot(Fsum,'.-b')
hold on
xlabel('t/s');
ylabel('U(t)/v');
title('Potential - Step');
plot(start,Fsum(start),'*g',start+step,Fsum(start+step),'*g');

%save 'G:\graduation_project\simulation\Xsum' Xsum;





