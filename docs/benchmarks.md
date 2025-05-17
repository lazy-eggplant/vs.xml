`pugixml` is considered as baseline. These results are preliminary and will be replaced with a more formal benchmark suite, they are just intended to manage expectations correctly.  

## Parsing XML

Moving from XML to the memory representation on the [big dataset nasa_10_f_bs](../assets/local/demo-0.xml), skipping escaping/de-escaping (`raw_strings` true) and without managing symbols (`EXTERN_REL`), is around 0.45x the base throughput of `pugixml`.  
Different configurations, like `COMPRESSED_ALL` or even just `OWNED` are clearly going to be more expensive (twice as slow for the worst case observed compared to `EXTERN_REL`), but reducing the default types size has a positive impact (between 20 and 30% improvement observed over the original 0.45x).

This is to be expected, as this library handles namespaces, has a higher memory footprint by default, and is forced to work with relative pointers throughout, which require some additional computation.  
So the slowdown is pretty tame overall, for what is intended as a rare operation. The expectation is that, once preprocessed, only the binary version of the original XML is going to be used.

By comparison, loading the binary version is orders of magnitude faster due to the lazy loading as a memory mapped file. This is kind of cheating, but pugixml does not have a comparable feature, so I cannot do much about that.  
Even forcing a workload which requires access to the full buffer, this library when loading the binary is about 1.2 times the base pugixml speed. This is in a worst-case memory footprint, reducing the default data types will improve performance as disk access is reduced.  

A more valid measure of performance will depend on the workload, and is going to be considered in a separate section.

## Saving as XML

Unsurprisingly, this is where `vs.xml` is weak by comparison. It is around 10% of pugixml speed.  
This is mostly due to my poor implementation but also the underwhelming performance of `std::format`. Moving to `fmt::format` we are now around 27% of pugixml speed, which is quite a bump.  
This metric will be surely improved as there is no good reason for it to be this bad, but XML serialization is not meant as a common operation for the scope of the library, so this result is not very impactful.