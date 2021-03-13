import java.util.Scanner;
import java.io.FileNotFoundException;
import java.lang.String;
import java.io.FileInputStream;

public class Fractions { 
  public static void main(String [] args){
  //Reads file containing fractions
    Scanner inputFile = null;
    try {
      inputFile = new Scanner(new FileInputStream("fractions.txt"));
    }
    catch (FileNotFoundException e) 
    {
      System.out.println("File not found.");
      System.exit(0);
    }

    //variables
    String[] fractions = new String[100]; //will take in the fractions
    String[] split = new String[2]; //used to split the fractions
    int[] numerator = new int[100]; //store numerators
    int[] denominator = new int[100]; //store denominators
    int count = 0; //number of lines in file
    int duplicate = 0; //number of fractions that are the duplicate
    boolean[] counted = new boolean[100];

    //count the number of lines in the file, put each line into the string[] fractions

    for (int i = 0; inputFile.hasNextLine(); i++){
      fractions[i]=inputFile.nextLine();
      count++;
    }

    //split the fractions[] into two arrays: numerator and denominator
    for(int i = 0; i < count; i++){
      split = fractions[i].split("/");
      numerator[i] = Integer.parseInt(split[0]);
      denominator[i] = Integer.parseInt(split[1]);
    }

    //used to compare specific numerator and denominator to the rest of the numbers
    int num;
    int den;

    //start off by comparing denominator, and then compares the numerator
    //of like denominators
    for(int i = 0; i < count; i++){
      den = denominator[i];
      num = numerator[i];
      for(int a = 0; a < count; a++){
        if (!counted[a]) {
          if(den == denominator[a]){ //compare denominators
            if(num == numerator[a]){ //compare numerators
              duplicate++;
              counted[a] = true;
            } 
          }
        }
      }
      if (duplicate>0) {
        System.out.println(num + "/" + den + " has a count of "+ duplicate);
      }
      duplicate = 0;
    }
  }
} 