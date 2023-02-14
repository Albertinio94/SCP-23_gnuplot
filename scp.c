#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *filenamefortime = "tempfileforgnu0";
char *filenameforspeedup = "tempfileforgnu1";
char *filenameforefficiency = "tempfileforgnu2";
char *time_png = "time.png";
char *speedup_png = "speedup.png";
char *efficiency_png = "efficiency.png";

void process_file(char *file_name);
void plot_file(char *input_file_name, char *output_file_name);
void delete_temp_files();


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 1;
    }
    process_file(argv[1]);
    plot_file(filenamefortime, time_png);
    plot_file(filenameforspeedup, speedup_png);
    plot_file(filenameforefficiency, efficiency_png);
    delete_temp_files();

    return 0;
}



void delete_temp_files()
{
    remove(filenamefortime);
    remove(filenameforspeedup);
    remove(filenameforefficiency);
}

void plot_file(char *input_file_name, char *output_file_name)
{
    FILE *gnuplotPipe;

    gnuplotPipe = popen("gnuplot", "w");

    fprintf(gnuplotPipe, "set term png\n");
    fprintf(gnuplotPipe, "set output '%s'\n", output_file_name);
    fprintf(gnuplotPipe, "plot '%s' with linespoints\n", input_file_name);

    pclose(gnuplotPipe);

}
unsigned int get_line_count(FILE *filepointer)
{
    char character = 'a';
    unsigned int count = 0;
    while (character != EOF)
    {
        character = fgetc(filepointer);
        if (character == '\n')
        {
            count++;
        }
    }
    return count;
}

unsigned int parse_first_line(FILE *filepointer)
{
    char *line = NULL;
    size_t size = 2048;
    unsigned int count = 0;
    char character;
    int i = 1;

    line = (char *)malloc(size * sizeof(char));
    character = line[0];
    getline(&line, &size, filepointer);
    while (character != '\n')
    {
        if (character == '\t')
        {
            count++;
        }
        character = line[i];
        i++;
    }
    free(line);

    return count;
}
unsigned int get_number_of_processors(FILE *filepointer)
{
    char number_of_processors[50];
    char end = 0;
    unsigned int i = 0;
    while (!end)
    {
        number_of_processors[i] = fgetc(filepointer);
        if (number_of_processors[i] == '\t')
        {
            end = 1;
        }
        else
            i++;
    }
    number_of_processors[i] = '\0';
    return atoi(number_of_processors);
}
float calculate_mean_line_by_line(FILE *filepointer, unsigned int number_of_runs)
{
    float sum = 0;
    float mean = 0;
    char time_of_run[50];
    unsigned int i = 0;
    char end = 0;
    while (!end)
    {
        time_of_run[i] = fgetc(filepointer);
        if (time_of_run[i] == '\t')
        {
            time_of_run[i] = '\0';
            sum += atof(time_of_run);
            i = 0;
        }
        else if (time_of_run[i] == '\n' || time_of_run[i] == EOF)
        {
            end = 1;
        }
        else
            i++;
    }
    time_of_run[i] = '\0';
    sum += atof(time_of_run);
    mean = sum / (float)number_of_runs;
    return mean;
}

void process_file(char *file_name)
{
    FILE *filepointer, *gnufilefortime, *gnufileforspeedup, *gnufileforefficiency;
    unsigned int number_of_runs, number_of_lines;
    float *means;
    unsigned int *number_of_processors;
    float speedup;
    filepointer = fopen(file_name, "r");

    number_of_lines = get_line_count(filepointer) - 1;
    rewind(filepointer);
    number_of_runs = parse_first_line(filepointer);
    means = (float *)malloc(number_of_lines * sizeof(float));
    number_of_processors = (unsigned int *)malloc(number_of_lines * sizeof(unsigned int));
    for (int i = 0; i < number_of_lines; i++)
    {
        number_of_processors[i] = get_number_of_processors(filepointer);
        means[i] = calculate_mean_line_by_line(filepointer, number_of_runs);
    }
    fclose(filepointer);
    gnufilefortime = fopen(filenamefortime, "w");
    gnufileforspeedup = fopen(filenameforspeedup, "w");
    gnufileforefficiency = fopen(filenameforefficiency, "w");
    for (int i = 0; i < number_of_lines; i++)
    {
        fprintf(gnufilefortime, "%i\t%f\n", number_of_processors[i], means[i]);
        speedup = means[0] / means[i];
        fprintf(gnufileforspeedup, "%i\t%f\n", number_of_processors[i], speedup);
        fprintf(gnufileforefficiency, "%i\t%f\n", number_of_processors[i], speedup / (float)number_of_processors[i]);
    }
    fclose(gnufilefortime);
    fclose(gnufileforspeedup);
    fclose(gnufileforefficiency);
    free(means);
    free(number_of_processors);
}