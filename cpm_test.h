
extern "C"
{
	const char *cpm_ls_test(char image[], char format[], char pat[], int *gargc, char ***gargv);
    const char *cpm_fmt_test(int *gargc, char ***gargv);
    const char *cpm_to_win_test(char image[], char format[], char *src, char *dest);
    const char *win_to_cpm_test(char image[], char format[], char *src, char *dest);
    const char *cpm_rm_test(char image[], char format[], char *src);
    const char *creat_cpm_test(char image[], char format[], char **boot);

}
