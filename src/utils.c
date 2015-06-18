//
// Created by Oleg Kisselef (olegkisselef at gmail dot com) on 6/17/15
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

void usage()
{
    printf("cctranslate is a part of ccextractor project http://ccextractor.org/\n");
    printf("version %d.%d.%d\n", CCTRANSLATE_VERSION_MAJOR,
           CCTRANSLATE_VERSION_MINOR, CCTRANSLATE_VERSION_MAINT);
    printf("usage: cctranslate args\n");
    printf("args syntax: --name=value, -n=value, --novaluearg\n");
    printf("\t-i, --input\t\tSource subrip filename]\n");
    printf("\t-o, --output\t\tOutput filename\n");
    printf("\t-k, --key\t\tGoogle Translate API key\n");
    printf("\t-l, --langs\t\tSet target languages (comma-separated, e.g. -l=fr,it)\n");
    printf("\t--list-langs\t\tList available languages\n");
    printf("\t\n");
    printf("\t-h, --help\t\tShow this help\n");
}

int has_extension(char *filename, char *ext)
{
    if (!filename) {
        fprintf(stderr, "ends_with_srt: filename is NULL");
        return 0;
    }
    if (!ext) {
        fprintf(stderr, "ends_with_srt: ext is NULL");
        return 0;
    }

    size_t fname_len = strlen(filename);
    size_t ext_len = strlen(ext);

    if (fname_len <= ext_len) {
        return 0;
    }
    if (!strcmp(filename + fname_len - ext_len, ext)) {
        return 1;
    }
    return 0;
}

char *compose_filename(char *filename, char *lang)
{
    size_t filename_len = strlen(filename);
    size_t lang_len = strlen(lang);
    size_t out_len = (lang_len + filename_len
                      + 1 //'.' before lang name
                      + 4 //".srt"
                      + 1 //'\0'
                     ) * sizeof(char);

    char *out = (char *) malloc(out_len);
    if (!out) {
        perror("get_filename: malloc() failed");
        return NULL;
    }
    memset(out, 0, out_len);

    if (has_extension(filename, ".srt") || has_extension(filename, ".SRT")) {
        strncpy(out, filename, filename_len - strlen(".srt"));
        strcat(out, ".");
        strcat(out, lang);
        strcat(out, ".srt");
    } else {
        strcpy(out, filename);
        strcat(out, ".");
        strcat(out, lang);
        strcat(out, ".srt");
    }

    return out;
}

cct_status curl_response_init(curl_response *response)
{
    response->len = 0;
    response->buf = malloc(response->len + 1);
    if (!response->buf) {
        fprintf(stderr, "curl_response_init: malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    response->buf[0] = '\0';
}

cct_status curl_response_cleanup(curl_response *response)
{
    free(response->buf);
    response->buf = NULL;
    response->len = 0;
}

char *curl_response_content(curl_response *response)
{
    return strdup(response->buf);
}

size_t on_curl_data_recieved(char *ptr, size_t size, size_t nmemb, curl_response *response)
{
    size_t new_len = response->len + size * nmemb;
    response->buf = realloc(response->buf, new_len + 1);
    if (!response->buf) {
        fprintf(stderr, "on_data_portion_recieved: malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(response->buf + response->len, ptr, size * nmemb);
    response->buf[new_len] = '\0';
    response->len = new_len;
    return size * nmemb;
}

/**
 * cJSON returns string value surrunded with double quotes, so
 * we have to remove them to use the string
 */
char *cjson_remove_quotes(char *string)
{
    size_t len = strlen(string);
    char *result = malloc((len - 1) * sizeof(char));
    if (!result) {
        fprintf(stderr, "cjson_remove_quotes: malloc() result failed\n");
        exit(EXIT_FAILURE);
    }
    strncpy(result, string + 1, len - 2);
    result[len - 2] = '\0';

    return result;
}