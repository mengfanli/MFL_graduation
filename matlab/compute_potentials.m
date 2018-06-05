function [ Usum ] = compute_potentials (X,Xsum,k,m,n,Po,a)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
Rat=(X(1)-Xsum(1,1))^2+(X(2)-Xsum(1,2))^2;%路径点和目标的距离平方
rat=sqrt(Rat);%路径点和目标的距离
for i=1:n
Rrei(i)=(X(1)-Xsum(i+1,1))^2+(X(2)-Xsum(i+1,2))^2;%路径点和障碍的距离平方
rre(i)=sqrt(Rrei(i));%路径点和障碍的距离保存在数组rrei中
R0=(Xsum(1,1)-Xsum(i+1,1))^2+(Xsum(1,2)-Xsum(i+1,2))^2;
r0=sqrt(R0);
if rre(i)>Po%如果每个障碍和路径的距离大于障碍影响距离，斥力令为0
Urer(i)=0;
else
Urer(i)=0.5*m*(1/rre(i)-1/Po)^2*(rat^a);
end
end
Uatt=0.5*k*Rat;
Usum=sum(Urer)+Uatt;

