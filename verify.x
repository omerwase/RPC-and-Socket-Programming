struct verify_param {
	int n;
	int l;
	int m;
};

program VERIFY {
	version VERIFY_VER {
		int RPC_INITVERIFYSERVER (verify_param) = 1;
		string RPC_GETSEG(int) = 2;
		string RPC_GET_S() = 3;
	} = 1;
} = 0x20304052;
