//***************************************************************************
int   MissileAnims[14];
CoolF14MissileAnims()
{
   int   i;

   for (i=1;i<12;i++)
      {
      if ((coolobjs[i].NUM!=-1)&&(collobj[i].ZOFF!=0))
         {
         MissileAnims[i]=0;
         switch (coolobjs[i].NUM)
            {
            case SDWNDR:
               MissileAnims[i]=1;
               if ((coolobj[i].Z-= (2*Fticks))<-100)
                  coolobj[i].NUM=-1;
               break;
            case FUELTANK:
               if ((coolobj[i].YOFF-=(Fticks/2))<-100)
                  coolobj[i].NUM=-1;
               break;
            default:
               if (coolobj[i].YOFF<-10)
                  {
                  MissileAnims[i]=1;
                  if ((coolobj[i].Z-= (2*Fticks))<-100)
                     coolobj[i].NUM=-1;
                  }
               else
                  {
                  coolobj[i].YOFF-=(Fticks/2);
                  }
               break;
            }
         }
      }
}
