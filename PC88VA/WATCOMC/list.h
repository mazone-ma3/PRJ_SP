/* ���X�g�\����`�}�N�� �w�b�_�t�@�C�� By ma3 2001. */

#ifndef LIST_H_INCLUDE
#define LIST_H_INCLUDE

/*int next_num[MAX + 1];*/	/* ���̗v�f�̔ԍ� */
/*int free_num[MAX];	*/	/* ���g�p�̗v�f�̔ԍ� */

#define END_LIST 255

#define INIT_LIST(MAX, INDEX, NEXT, FREE) {\
	{\
		for(INDEX = 0; INDEX <= MAX; INDEX++){\
			FREE[INDEX] = INDEX + 1;\
			NEXT[INDEX] = END_LIST;\
		}\
		FREE[MAX - 1] = END_LIST;\
		FREE[MAX] = 0;\
	}\
}

/* ���X�g�̐擪�ɗv�f��ǉ����� */
/* (�t���[���X�g����v�f���폜����) */

/* �t���[���X�g�̐擪����v�f�����o�� */
/* �t���[���X�g�̐擪����v�f���폜���� */
/*  */
/* ���X�g�̐擪�Ɏ��o�����v�f��ǉ����� */

#define ADD_LIST(MAX, NEW_INDEX, NEXT, FREE) {\
	NEW_INDEX = FREE[MAX];\
	FREE[MAX] = FREE[NEW_INDEX];\
	NEXT[NEW_INDEX] = NEXT[MAX];\
	NEXT[MAX] = NEW_INDEX;\
}

/* ���X�g����v�f���폜���� */
/* (�t���[���X�g�ɗv�f��ǉ�����) */
/*  */
/* �t���[���X�g�̐擪�ɗv�f��ǉ����� */
/* ���X�g����v�f���폜���� */
#define DEL_LIST(MAX, INDEX, PREV, NEXT, FREE) {\
	FREE[INDEX] = FREE[MAX];\
	FREE[MAX] = INDEX;\
	NEXT[PREV] = NEXT[INDEX];\
	INDEX = PREV;\
}

#define SEARCH_LIST(MAX, INDEX, NEXT) for(INDEX = NEXT[MAX]; INDEX != END_LIST; INDEX = NEXT[INDEX])

#define SEARCH_LIST2(MAX, INDEX, PREV, NEXT) for(INDEX = NEXT[MAX], PREV = MAX; INDEX != END_LIST; PREV = INDEX, INDEX = NEXT[INDEX])

#endif
