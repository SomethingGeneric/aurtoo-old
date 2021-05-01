```
char text[255];
FILE *name;
name = popen("whoami", "r");
fgets(text, sizeof(text), name);
cout << "Name is : " << text;
pclose(name);
cout << endl;
return 0;
```