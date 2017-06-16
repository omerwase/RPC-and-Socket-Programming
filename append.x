struct append_param {
	int f;
	int n;
	int l;
	int m;
	char c[3];
	string host_name2<>;
	int maxFactor[2];
};

program APPEND {
	version APPEND_VER {
		int RPC_INITAPPENDSERVER (append_param) = 1;
		int RPC_APPEND(char) = 2;
	} = 1;
} = 0x20304051;
