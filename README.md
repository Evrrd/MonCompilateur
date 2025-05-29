# CERIcompiler

A simple compiler.
From : Pascal-like imperative LL(k) langage
To : 64 bit 80x86 assembly langage (AT&T)


# Liste des codes réalisés :

> Je suis pas sûr à 100% donc si y'a un code identique au votre
> qui se trouve dans ma liste, c'est que j'ai oublié.


- Factor();
- Term();
- SimpleExpression();
- VarDeclaration();
- AssignementStatement();
- ForStatement();
- BlockStatement();
- DisplayStatement(); (j'ai essayé de le faire tout seul mais j'ai eu beaucoup de soucis avec la partie double)
- CaseStatement(); (J'ai pas réussi à faire en sorte qu'on puisse mettre plusieurs CaseLabel, mais sinon ça fonctionne)
- RepeatStatement();
- Statement();

> Sinon dans tokeniser.l j'ai très peu copié à part si ça marchait vraiment pas, par exemple mon charconst ne marchait pas. 