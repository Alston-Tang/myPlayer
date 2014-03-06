using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Threading;

namespace myPlayer
{
    class WavePlayer
    {
        [DllImport("myPlayerDll",CharSet=CharSet.Unicode)]
        public extern static int test();
        [DllImport("myPlayerDLL", CharSet = CharSet.Unicode)]
        public extern static int play();
        [DllImport("myPlayerDLL", CharSet = CharSet.Unicode)]
        public extern static int stop();
        [DllImport("myPlayerDLL", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public extern static int loadFile(string filePath);
        [DllImport("myPlayerDLL", CharSet = CharSet.Unicode)]
        public extern static int init();
    }

    class Program
    {
        static void haha()
        {
            WavePlayer.init();
            WavePlayer.loadFile("D:\\3280\\pro\\1.wav");
            WavePlayer.play();
        }
        static void Main(string[] args)
        {

            Thread test = new Thread(new ThreadStart(myPlayer.Program.haha));
            test.Start();

            Console.Write("HaHa\n");
            Console.Write(WavePlayer.test());
           
            System.Threading.Thread.Sleep(500);

            WavePlayer.stop();

            
        }
    }


}
