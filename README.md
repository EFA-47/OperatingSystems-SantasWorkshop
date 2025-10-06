Operating Systems Project (C, POSIX Threads) 
 
Gerçek zamanlı, çok iş parçacıklı (multithreaded) bir atölye simülasyonu geliştirildi.Pthread semaforları, mutex ve koşul değişkenleri (condition variables) kullanarak deadlock-free bir görev planlayıcı (scheduler) tasarlandı. 
Elfler ve Santa arasında görev paylaşımı yapılarak boyama, montaj, paketleme, kalite kontrol (QA) ve teslimat süreçleri paralel yürütüldü. 
Starvation önleme algoritması uygulanarak QA görevlerinin gecikmesi engellendi. 
Gerçek zamanlı loglama sistemi geliştirildi; her görev için işlem süresi, görev tipi, işçi (Elf/Santa) ve görev sırası kaydedildi. 
Komut satırı argümanlarıyla simülasyon süresi ve snapshot periyodu dinamik olarak ayarlanabildi. 
