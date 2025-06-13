> [!WARNING]  
> Not implemented yet, just basic specs to sketch a new feature built on top of queries (and to build queries).

Queries as they are right now are flexible, but not optimal for all applications:

- They cannot be easily serialized and de-serialized as they involve `std::function` in the variadic.
- They cannot be easily hashed for the reason above, and because they are made of many split memory segments.
- The lack of a quick way to hash makes the process of caching queries impractical.

This proposal introduces a flat-tree representation for queries, which are going to perform better in those specific instances.  
They will be unable to store lambdas, but it is always possible to add some kind of scripting capability I guess (downstream, I don't think it is good to have that in here).

Unlike the tree navigation of XML, where being able to go back to parent with no further contextual information is important, queries are always directed downwards.  
Also, aside for alignment purposes, there is no good reason to leave strings in a separate table, and they are much more less likely to be repeated.  
So having a split structural and symbol table might not be as beneficial.  


Still, a layout of fully regular cells seems quite a good idea to me.

Sketching layout or something.

```
BEGIN(3) OP OP OP 
    BEGIN(0)
        BEGIN(1) OP
        END
        BEGIN(2) OP OP
        END
    END
    BEGIN(0)
        BEGIN(1) OP
        END
    END
END
EOQ #end of query

```
Extra operation `END` used to mark end of branch and trace back.

Operands are only string_views, details about their interpretation is left to the BEGIN block.
END blocks only cover an offset to the parent, to ensure navigation back is possible. 
Fully linear structures can avoid storing end tags, as they will never be used.


```
struct token_base{
    operation_t op;     //5 bits
    size_t operands;    //~~4~~ 3 bits

    operands type = 8 * 2 bit = 16bit.

    payload/alignment   //WORD-8bit
};

struct operand_t{
    data_kind_t kind;
    std::string_view data;
}

```
