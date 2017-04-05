#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char a[] = "[\"fds\",\"fds\" , 0.0, \"fdaf\", -1, \"fdsaf\"]";
char b[] = "[\"CA16C80793B0493F\", false, \"656FBE66A9094892\", \"d78bbc67\", \"\"]";

char *oa_strncpy(char *dest, const char *src, size_t n)
{
	if (!dest || !src || n < 0) {
		return NULL;
	}
	strncpy(dest, src, n);
	dest[n - 1] = '\0';
	return dest;
}

char *oa_strchr(const char *base, char dest)
{
	if (!base) {
		return NULL;
	}
	return strchr(base, dest);
}

void *oa_malloc(size_t size)
{
	if (size <= 0) {
		return NULL;
	}
	return malloc(size);
}

char *oa_strstr(const char *base, const char *dest)
{
	if (!base || !dest) {
		return NULL;
	}
	return strstr(base, dest);
}


char* oa_skip_space_and_d_qutation(char *start_ptr)
{
	while (*start_ptr == ' ') {
		start_ptr++;
	}
	if (*start_ptr == '"') {
		start_ptr++;
	}
	return start_ptr;
}

char* oa_back_skip_space_and_d_quation(char *end_ptr)
{
	while (*end_ptr == ' ') {
		end_ptr--;
	}
	if (*end_ptr == '"') {
		end_ptr--;
	}

	return end_ptr;
}


int	seperator_trans_to_seg(char *trans, char ***trans_seg)
{
	char *curr_trans_pos, *start_pos, *end_pos, *comma_pos;
	int trans_iterator, trans_seg_size;

	curr_trans_pos = trans + 1;
	trans_iterator = 0;
	while ((comma_pos = oa_strchr(curr_trans_pos, ',')) != NULL) {
		start_pos = oa_skip_space_and_d_qutation(curr_trans_pos);
		end_pos = oa_back_skip_space_and_d_quation(comma_pos - 1);
		trans_seg_size = end_pos - start_pos + 2;
		(*trans_seg)[trans_iterator] = (char*)oa_malloc(trans_seg_size);
		oa_strncpy((*trans_seg)[trans_iterator], start_pos, trans_seg_size);
		trans_iterator++;
		curr_trans_pos = comma_pos + 1;
	}

	end_pos = oa_strchr(curr_trans_pos, ']');
	start_pos = oa_skip_space_and_d_qutation(curr_trans_pos);
	end_pos = oa_back_skip_space_and_d_quation(end_pos - 1);
	trans_seg_size = end_pos - start_pos + 2;
	(*trans_seg)[trans_iterator] = (char*)oa_malloc(trans_seg_size);
	oa_strncpy((*trans_seg)[trans_iterator], start_pos, trans_seg_size);

	return trans_iterator + 1;

}

void main()
{
	char **trans_seg;
	char *callee_cross_process_id = NULL;
	char *callee_transaction_name = NULL;
	char *callee_guid = NULL;
	char *tran_seg_ptr;
	double callee_duration;
	int seg_num;

	trans_seg = (char**)oa_malloc(sizeof(char*) * 6);
	seg_num = seperator_trans_to_seg(a, &trans_seg);

	callee_cross_process_id = trans_seg[0];

	callee_transaction_name = trans_seg[1];

	callee_duration = strtod(trans_seg[3], NULL);

	callee_guid = trans_seg[5];

	printf("CURL: callee_cross_process_id :%s", callee_cross_process_id);
	printf("CURL: callee_transaction_name :%s", callee_transaction_name);
	printf("CURL: callee_guid :%s", callee_guid);
	printf("CURL: callee_duration :%f", callee_duration);
	printf("%d\n", seg_num);

	int trans_iterator;
	for (trans_iterator = 0; trans_iterator < seg_num; trans_iterator++) {
		tran_seg_ptr = trans_seg[trans_iterator];
		free(tran_seg_ptr);
	}
	free(trans_seg);



	trans_seg = (char**)oa_malloc(sizeof(char*) * 5);
	seg_num = seperator_trans_to_seg(b, &trans_seg);

	char* refer_id = trans_seg[0];
	printf("referer_guid is :%s", refer_id);

	int is_force_trace;

	if (_strnicmp(trans_seg[1], "true", 4)) {
		is_force_trace = 1;
	}
	else {
		is_force_trace = 0;
	}
	printf("is force trace :%d", is_force_trace);


	char* trip_id = trans_seg[2];
	printf("trip_id is :%s", trip_id);

	for (trans_iterator = 0; trans_iterator < seg_num; trans_iterator++) {
		free(trans_seg[trans_iterator]);
	}
	free(trans_seg);

}